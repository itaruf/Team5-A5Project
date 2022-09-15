/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "WaapiRenameWatcher.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkWaapiUtils.h"
#include "Async/Async.h"
#include "AkSettingsPerUser.h"
#include "IAudiokineticTools.h"

#define LOCTEXT_NAMESPACE "AkWaapiNameSynchronizer"

namespace WaapiNameSynchronizer_Helper
{
	void SubscribeWaapiCallback(FAkWaapiClient* WaapiClient, const char* Uri, WampEventCallback Callback, uint64& SubscriptionId, const TArray<FString>& ReturnArgs)
	{
		if (SubscriptionId)
			return;

		TArray<TSharedPtr<FJsonValue>> ReturnArgsArray;
		for (auto& arg : ReturnArgs)
			ReturnArgsArray.Add(MakeShared<FJsonValueString>(arg));

		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		Options->SetArrayField(WwiseWaapiHelper::RETURN, ReturnArgsArray);

		TSharedPtr<FJsonObject> result;
		WaapiClient->Subscribe(Uri, Options, Callback, SubscriptionId, result);
	}

	void UnsubscribeWaapiCallback(FAkWaapiClient* WaapiClient, uint64& SubscriptionId)
	{
		if (SubscriptionId == 0)
			return;

		TSharedPtr<FJsonObject> result = MakeShared<FJsonObject>();
		WaapiClient->Unsubscribe(SubscriptionId, result);
		SubscriptionId = 0;
	}
}

WaapiRenameWatcher& WaapiRenameWatcher::Get()
{
	static WaapiRenameWatcher instance;
	return instance;
}

void WaapiRenameWatcher::Init()
{
	if (bInitialized)
	{
		return;
	}

	const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
	AkSettingsPerUser->OnAutoConnectToWaapiChanged.AddLambda([this]()
		{
			const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
			if (AkSettingsPerUser->bAutoConnectToWAAPI)
			{
				BindToWaapiClient();
				SubscribeToWaapiRename();
			}
			else
			{
				UnsubscribeFromWaapiRename();
				UnbindFromWaapiClient();
			}
		});

	FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Cannot get Waapi client, asset name sync disabled"));
		bInitialized = false;
		return;
	}

	BindToWaapiClient();
	SubscribeToWaapiRename();

	bInitialized = true;
}

void WaapiRenameWatcher::UnInit()
{
	if(!bInitialized)
	{
		return;
	}
	UnsubscribeFromWaapiRename();
	UnbindFromWaapiClient();
	bInitialized = false;
}

void WaapiRenameWatcher::BindToWaapiClient()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (WaapiClient)
	{
		ProjectLoadedHandle = WaapiClient->OnProjectLoaded.AddRaw(this, &WaapiRenameWatcher::SubscribeToWaapiRename);
		ConnectionLostHandle = WaapiClient->OnConnectionLost.AddRaw(this, &WaapiRenameWatcher::UnsubscribeFromWaapiRename);
		ClientBeginDestroyHandle = WaapiClient->OnClientBeginDestroy.AddRaw(this, &WaapiRenameWatcher::UnsubscribeFromWaapiRename);
	}
}

void WaapiRenameWatcher::UnbindFromWaapiClient()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (WaapiClient)
	{
		WaapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
		WaapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
		WaapiClient->OnClientBeginDestroy.Remove(ClientBeginDestroyHandle);
	}

	ProjectLoadedHandle.Reset();
	ConnectionLostHandle.Reset();
	ClientBeginDestroyHandle.Reset();
}

void WaapiRenameWatcher::SubscribeToWaapiRename()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Cannot get Waapi client, asset sync disabled"));
		return;
	}

	const TArray<FString> ReturnArgs
	{
		WwiseWaapiHelper::ID,
		WwiseWaapiHelper::NAME,
		WwiseWaapiHelper::PATH,
		WwiseWaapiHelper::TYPE,
		WwiseWaapiHelper::PARENT,
	};
#if AK_SUPPORT_WAAPI
	WaapiNameSynchronizer_Helper::SubscribeWaapiCallback(WaapiClient,
		ak::wwise::core::object::nameChanged,
		WampEventCallback::CreateRaw(this, &WaapiRenameWatcher::OnWaapiRenamed),
		IdRenamed,
		ReturnArgs);
#endif

}

void WaapiRenameWatcher::UnsubscribeFromWaapiRename()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Cannot unsubscribe from asset sync, Waapi client not available"));
		return;
	}

	WaapiNameSynchronizer_Helper::UnsubscribeWaapiCallback(WaapiClient, IdRenamed);
}

void WaapiRenameWatcher::OnWaapiRenamed(uint64_t Id, TSharedPtr<FJsonObject> Response)
{
	AsyncTask(ENamedThreads::GameThread, [this, Response]
		{
			const TSharedPtr<FJsonObject>* ResultObjectPtr = nullptr;
			if (!Response->TryGetObjectField(WwiseWaapiHelper::OBJECT, ResultObjectPtr) || !ResultObjectPtr)
				return;
			auto resultObject = *ResultObjectPtr;

			FString AssetId;
			if (!resultObject->TryGetStringField(WwiseWaapiHelper::ID, AssetId))
				return; // error parsing Json

			FGuid GuidId;
			FGuid::ParseExact(AssetId, EGuidFormats::DigitsWithHyphensInBraces, GuidId);

			FString OldName;
			Response->TryGetStringField(WwiseWaapiHelper::OLD_NAME, OldName);

			FString NewName;
			Response->TryGetStringField(WwiseWaapiHelper::NEW_NAME, NewName);

			// Invalid old name or name is not changed.
			if (OldName.IsEmpty() || NewName.IsEmpty() || OldName == NewName || !GuidId.IsValid())
			{
				UE_LOG(LogAudiokineticTools, Verbose, TEXT("Ignoring rename of Wwise asset %s from %s to %s"), *AssetId, *OldName, *NewName);
				return;
			}

			FString RelativePath;

			OnWaapiAssetRenamedMultiDelegate.Broadcast(GuidId, NewName, RelativePath);
		});
}

#undef LOCTEXT_NAMESPACE
