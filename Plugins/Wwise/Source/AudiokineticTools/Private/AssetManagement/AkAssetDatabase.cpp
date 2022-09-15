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

#include "AssetManagement/AkAssetDatabase.h"
#include "AkAudioType.h"
#include "AkAcousticTexture.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkUnrealHelper.h"
#include "AkAudioDevice.h"
#include "AkSettingsPerUser.h"
#include "IAudiokineticTools.h"
#include "WaapiRenameWatcher.h"

#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Async/Async.h"
#include "Misc/FeedbackContext.h"
#include "ObjectTools.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "AkAudio"

AkAssetDatabase& AkAssetDatabase::Get()
{
	static AkAssetDatabase instance;
	return instance;
}

AkAssetDatabase::AkAssetDatabase()
{
	AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
}

void AkAssetDatabase::Init()
{
	if ( const UAkSettingsPerUser* UserSettings = GetDefault<UAkSettingsPerUser>())
	{
		if (UserSettings->bAutoSyncWwiseAssetNames )
		{
			auto& WaapiNameSync = WaapiRenameWatcher::Get();
			RenameAssetHandle = WaapiNameSync.OnWaapiAssetRenamedMultiDelegate.AddRaw(this, &AkAssetDatabase::OnRenameAsset);
		}

		UserSettings->OnAutoSyncWwiseAssetNamesChanged.AddLambda([this]()
		{
			const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
			if (AkSettingsPerUser && AkSettingsPerUser->bAutoSyncWwiseAssetNames)
			{
				BindToWaapiRename();
			}
			else
			{
				UnbindFromWaapiRename();
			}
		});
	}
}

void AkAssetDatabase::BindToWaapiRename()
{
	UnbindFromWaapiRename();
	auto& WaapiNameSync = WaapiRenameWatcher::Get();
	RenameAssetHandle = WaapiNameSync.OnWaapiAssetRenamedMultiDelegate.AddRaw(this, &AkAssetDatabase::OnRenameAsset);
}

void AkAssetDatabase::UnbindFromWaapiRename()
{
	if (RenameAssetHandle.IsValid())
	{
		auto& WaapiNameSync = WaapiRenameWatcher::Get();
		WaapiNameSync.OnWaapiAssetRenamedMultiDelegate.Remove(RenameAssetHandle);
		RenameAssetHandle.Reset();
	}
}

bool AkAssetDatabase::FindAllAssets(TArray<FAssetData>& OutData)
{
	AssetRegistryModule->Get().GetAssetsByClass(UAkAudioType::StaticClass()->GetFName(), OutData, true);
	return OutData.Num() > 0;
}

bool AkAssetDatabase::FindAssets(const FGuid& AkGuid, TArray<FAssetData>& OutData)
{
	TMultiMap<FName, FString> Search;
	Search.Add(GET_MEMBER_NAME_CHECKED(UAkAudioType, WwiseGuid), AkGuid.ToString(EGuidFormats::Digits));
	AssetRegistryModule->Get().GetAssetsByTagValues(Search, OutData);

	return OutData.Num() > 0;
}

bool AkAssetDatabase::FindAssets(const FString& AssetName, TArray<FAssetData>& OutData)
{
	TMultiMap<FName, FString> Search;
	Search.Add(GET_MEMBER_NAME_CHECKED(FAssetData, AssetName), AssetName);
	AssetRegistryModule->Get().GetAssetsByTagValues(Search, OutData);

	return OutData.Num() > 0;
}

bool AkAssetDatabase::FindFirstAsset(const FGuid& AkGuid, FAssetData& OutAsset)
{
	TArray<FAssetData> Assets;
	if(FindAssets(AkGuid, Assets))
	{
		OutAsset = Assets[0];
		return true;
	}
	return false;
}

bool AkAssetDatabase::FindFirstAsset(const FString& AssetName, FAssetData& OutAsset)
{
	TArray<FAssetData> Assets;
	if(FindAssets(AssetName, Assets))
	{
		OutAsset = Assets[0];
		return true;
	}
	return false;
}

bool AkAssetDatabase::RenameAsset(const FGuid& Id, const FString& AssetName,
                                  const FString& RelativePath)
{
	check(IsInGameThread());

	auto parentPath = RelativePath;

	TArray<FAssetData> AssetData;
	if (!FindAssets(Id, AssetData))
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Can't find Wwise asset to rename with ID %s"), *Id.ToString());
		return false;
	}

	TArray<FAssetRenameData> AssetsToRename;

	for (FAssetData Asset : AssetData)
	{
		if (Asset.AssetName.ToString() != AssetName || parentPath != Asset.PackagePath.ToString())
		{
			if (parentPath.IsEmpty())
			{
				parentPath = Asset.PackagePath.ToString();
			}

			FAssetRenameData NewAssetRenameData(Asset.GetAsset(), parentPath, AssetName);
			AssetsToRename.Add(NewAssetRenameData);
			UE_LOG(LogAudiokineticTools, Verbose, TEXT("Renaming Wwise asset %s"), *AssetName);
		}
	}

	if (!AssetToolsModule->Get().RenameAssets(AssetsToRename))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to rename Wwise Assets"));
		return false;
	}
	
	return true;
}

void AkAssetDatabase::DeleteAsset(const FGuid& Id)
{
	check(IsInGameThread());

	TArray<FAssetData> AssetsToDelete;

	if (FindAssets(Id, AssetsToDelete))
	{
		ObjectTools::DeleteAssets(AssetsToDelete, true);
	}
}

void AkAssetDatabase::DeleteAssets(const TSet<FGuid>& AssetsId)
{

	for (auto& ID : AssetsId)
	{
		DeleteAsset(ID);
	}
}


void AkAssetDatabase::FixUpRedirectors(const FString& AssetPackagePath)
{
	TArray<UObjectRedirector*> redirectorsToFix;

	TArray<FAssetData> foundRedirectorsData;
	AssetRegistryModule->Get().GetAssetsByClass(UObjectRedirector::StaticClass()->GetFName(), foundRedirectorsData);

	if (foundRedirectorsData.Num() > 0)
	{
		for (auto& entry : foundRedirectorsData)
		{
			if (auto redirector = Cast<UObjectRedirector>(entry.GetAsset()))
			{
				if (redirector->DestinationObject)
				{
					auto pathName = redirector->DestinationObject->GetPathName();
					if (pathName.StartsWith(AssetPackagePath))
					{
						redirectorsToFix.Add(redirector);
					}
				}
			}
		}
	}

	if (redirectorsToFix.Num() > 0)
	{
		AssetToolsModule->Get().FixupReferencers(redirectorsToFix);
	}
}

bool AkAssetDatabase::IsAkAudioType(const FAssetData& AssetData)
{
	static const TArray<FName> AkAudioClassNames = {
		UAkAcousticTexture::StaticClass()->GetFName(),
		UAkAudioEvent::StaticClass()->GetFName(),
		UAkAuxBus::StaticClass()->GetFName(),
		UAkRtpc::StaticClass()->GetFName(),
		UAkStateValue::StaticClass()->GetFName(),
		UAkSwitchValue::StaticClass()->GetFName(),
		UAkTrigger::StaticClass()->GetFName()
	};

	if (AkAudioClassNames.Contains(AssetData.AssetClass))
		return true;

	return false;
}

void AkAssetDatabase::OnRenameAsset(const FGuid& Id, const FString& AssetName, const FString& RelativePath)
{
	RenameAsset(Id, AssetName, RelativePath);
}

bool AkAssetDatabase::CheckIfLoadingAssets()
{
	return AssetRegistryModule->Get().IsLoadingAssets();
}

void AkAssetDatabase::UnInit()
{
	UnbindFromWaapiRename();
}

#undef LOCTEXT_NAMESPACE
