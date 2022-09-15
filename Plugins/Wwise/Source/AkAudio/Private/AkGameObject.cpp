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


/*=============================================================================
	AkGameObject.cpp:
=============================================================================*/

#include "AkGameObject.h"
#include "AkAudioEvent.h"
#include "AkComponentCallbackManager.h"
#include "AkRtpc.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"

class FPostAssociatedEventAction : public FAkPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink = 0;
	FWeakObjectPtr CallbackTarget;
	int32* PlayingID = nullptr;
	TFuture<AkPlayingID> FuturePlayingID;
	UAkAudioEvent* AkEvent = nullptr;
	bool* bGameObjectStarted= nullptr;

	FPostAssociatedEventAction(const FLatentActionInfo& LatentInfo, int32* PlayingID, UAkAudioEvent* Event, bool* bStarted)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, PlayingID(PlayingID)
		, AkEvent(Event)
		, bGameObjectStarted(bStarted)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		bool futureIsReady = FuturePlayingID.IsReady();
		if (futureIsReady)
		{
			*PlayingID = FuturePlayingID.Get();
			if (bGameObjectStarted!=nullptr)
			{
				*bGameObjectStarted = true;
			}
		}

		Response.FinishAndTriggerIf(futureIsReady, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	virtual FString GetDescription() const override
	{
		return TEXT("Waiting for posted AkEvent to load media.");
	}
#endif
};

UAkGameObject::UAkGameObject(const class FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer)
{
	bStarted = false;
}

int32 UAkGameObject::PostAssociatedAkEvent(int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback)
{
	return PostAkEvent(AkAudioEvent, CallbackMask, PostEventCallback, EventName);
}


int32 UAkGameObject::PostAkEvent(class UAkAudioEvent * AkEvent, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback, const FString& InEventName)
{
	AkPlayingID playingID = PostAkEventByNameWithDelegate(AkEvent, InEventName, CallbackMask, PostEventCallback);
	return playingID;
}

AkPlayingID UAkGameObject::PostAkEventByNameWithDelegate(class UAkAudioEvent * AkEvent, const FString& InEventName, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback)
{
	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	auto AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		TArray<AkExternalSourceInfo> ExternalSources;
		if (AkEvent)
		{
			IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(ExternalSources, AkEvent->GetExternalSources());
		}
		playingID = AudioDevice->PostEventOnAkGameObject(AudioDevice->GetShortID(AkEvent, InEventName), this, PostEventCallback, CallbackMask, ExternalSources);
		if (playingID != AK_INVALID_PLAYING_ID)
			bStarted = true;
	}
	
	return playingID;
}

void UAkGameObject::PostAssociatedAkEventAsync(const UObject* WorldContextObject, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback, FLatentActionInfo LatentInfo, int32& PlayingID)
{
	PostAkEventAsyncByEvent(WorldContextObject, AkAudioEvent, CallbackMask, PostEventCallback, LatentInfo, PlayingID);
}

void UAkGameObject::PostAkEventAsync(const UObject* WorldContextObject,
	UAkAudioEvent* AkEvent,
	int32& PlayingID,
	int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback,
	FLatentActionInfo LatentInfo
)
{
	PostAkEventAsyncByEvent(WorldContextObject, AkEvent, CallbackMask, PostEventCallback, LatentInfo, PlayingID);
}

void UAkGameObject::PostAkEventAsyncByEvent(const UObject* WorldContextObject,
	class UAkAudioEvent* AkEvent,
	int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback,
	FLatentActionInfo LatentInfo,
	int32& PlayingID
)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	FLatentActionManager& LatentActionManager = DeviceAndWorld.CurrentWorld->GetLatentActionManager();
	FPostAssociatedEventAction* NewAction = LatentActionManager.FindExistingAction<FPostAssociatedEventAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	if (!NewAction)
	{
		NewAction = new FPostAssociatedEventAction(LatentInfo, &PlayingID, AkEvent, &bStarted);
		NewAction->FuturePlayingID = DeviceAndWorld.AkAudioDevice->PostAkAudioEventOnAkGameObjectAsync(AkEvent, this, PostEventCallback, CallbackMask);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
}

void UAkGameObject::SetRTPCValue(const UAkRtpc* RTPCValue, float Value, int32 InterpolationTimeMs, FString RTPC) const
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		if (RTPCValue)
		{
			SoundEngine->SetRTPCValue(RTPCValue->GameParameterCookedData.ShortId, Value, GetAkGameObjectID(), InterpolationTimeMs);
		}
		else
		{
			SoundEngine->SetRTPCValue(TCHAR_TO_AK(*RTPC), Value, GetAkGameObjectID(), InterpolationTimeMs);
		}
	}
}

void UAkGameObject::GetRTPCValue(const UAkRtpc* RTPCValue, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType, FString RTPC, int32 PlayingID) const
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		AK::SoundEngine::Query::RTPCValue_type RTPCType = (AK::SoundEngine::Query::RTPCValue_type)InputValueType;

		if (RTPCValue)
		{
			SoundEngine->Query.GetRTPCValue(RTPCValue->GameParameterCookedData.ShortId, GetAkGameObjectID(), PlayingID, Value, RTPCType);
		}
		else
		{
			SoundEngine->Query.GetRTPCValue(TCHAR_TO_AK(*RTPC), GetAkGameObjectID(), PlayingID, Value, RTPCType);
		}

		OutputValueType = (ERTPCValueType)RTPCType;
	}
}

void UAkGameObject::GetRTPCValue(FString RTPC, int32 PlayingID, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType) const
{
	GetRTPCValue(nullptr, InputValueType, Value, OutputValueType, RTPC, PlayingID);
}

bool UAkGameObject::VerifyEventName(const FString& InEventName) const
{
	const bool IsEventNameEmpty = InEventName.IsEmpty();
	if (IsEventNameEmpty)
	{
		FString OwnerName = FString(TEXT(""));
		FString ObjectName = GetName();

		const auto owner = GetOwner();
		if (owner)
			OwnerName = owner->GetName();

		UE_LOG(LogAkAudio, Warning, TEXT("[%s.%s] AkGameObject: Attempted to post an empty AkEvent name."), *OwnerName, *ObjectName);
	}

	return !IsEventNameEmpty;
}

bool UAkGameObject::AllowAudioPlayback() const
{
	UWorld* CurrentWorld = GetWorld();
	return (CurrentWorld && CurrentWorld->AllowAudioPlayback() && !IsBeingDestroyed());
}

AkGameObjectID UAkGameObject::GetAkGameObjectID() const
{
	return (AkGameObjectID)this;
}

void UAkGameObject::Stop()
{
	if (HasActiveEvents() && FAkAudioDevice::Get() && IsRegisteredWithWwise)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StopAll(GetAkGameObjectID());
		SoundEngine->RenderAudio();
	}
}

bool UAkGameObject::HasActiveEvents() const
{
	auto CallbackManager = FAkComponentCallbackManager::GetInstance();
	return (CallbackManager != nullptr) && CallbackManager->HasActiveEvents(GetAkGameObjectID());
}
