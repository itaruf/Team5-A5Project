/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

/*=============================================================================
	AkAudioClasses.cpp:
=============================================================================*/
#include "AkGameplayStatics.h"

#include "AkAmbientSound.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAudioType.h"
#include "AkComponent.h"
#include "AkEffectShareSet.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "Engine/GameEngine.h"
#include "EngineUtils.h"
#include "AkAcousticPortal.h"
#include "AkAuxBus.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/WwiseExternalSourceManager.h"

#include "inttypes.h"
#include "WwiseInitBankLoader/WwiseInitBankLoader.h"


bool UAkGameplayStatics::m_bSoundEngineRecording = false;

/*-----------------------------------------------------------------------------
	UAkGameplayStatics.
-----------------------------------------------------------------------------*/

UAkGameplayStatics::UAkGameplayStatics(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
}

class UAkComponent * UAkGameplayStatics::GetAkComponent( class USceneComponent* AttachToComponent, bool& ComponentCreated, FName AttachPointName, FVector Location, EAttachLocation::Type LocationType )
{
	if ( AttachToComponent == NULL )
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::GetAkComponent: NULL AttachToComponent specified!"));
		return NULL;
	}

	FAkAudioDevice * AkAudioDevice = FAkAudioDevice::Get();
	if( AkAudioDevice )
	{
		return AkAudioDevice->GetAkComponent( AttachToComponent, AttachPointName, &Location, LocationType, ComponentCreated );
	}

	return NULL;
}

bool UAkGameplayStatics::IsEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

bool UAkGameplayStatics::IsGame(UObject* WorldContextObject)
{
	EWorldType::Type WorldType = EWorldType::None;
	if (WorldContextObject)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if(World)
			WorldType = World->WorldType;
	}

	return WorldType == EWorldType::Game || WorldType == EWorldType::GamePreview || WorldType == EWorldType::PIE;
}

int32 UAkGameplayStatics::PostEvent(UAkAudioEvent* AkEvent, AActor* Actor, int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback, bool bStopWhenAttachedToDestroyed,
	FString EventName)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostOnActor(Actor, PostEventCallback, CallbackMask, bStopWhenAttachedToDestroyed);
	}

	AkDeviceAndWorld DeviceAndWorld(Actor);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		return AK_INVALID_PLAYING_ID;
	}
	
	AkCallbackType AkCallbackMask = AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask);
	AkUInt32 ShortId = DeviceAndWorld.AkAudioDevice->GetShortID(AkEvent, EventName);
	return DeviceAndWorld.AkAudioDevice->PostEventOnActor(ShortId, Actor, PostEventCallback, AkCallbackMask, bStopWhenAttachedToDestroyed, {});
}

int32 UAkGameplayStatics::PostAndWaitForEndOfEvent(UAkAudioEvent* AkEvent, AActor* Actor, bool bStopWhenAttachedToDestroyed,
	FLatentActionInfo LatentInfo)
{
	if (UNLIKELY(!IsValid(AkEvent)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post and wait invalid AkAudioEvent on actor '%s'."), IsValid(Actor) ? *Actor->GetName() : TEXT("(invalid)"));
		return AK_INVALID_PLAYING_ID;
	}

	return AkEvent->PostOnActorAndWait(Actor, bStopWhenAttachedToDestroyed, LatentInfo);
}

void UAkGameplayStatics::PostAndWaitForEndOfEventAsync(
	class UAkAudioEvent* AkEvent,
	class AActor* Actor,
	int32& PlayingID,
	bool bStopWhenAttachedToDestroyed,
	FLatentActionInfo LatentInfo
	)
{
	if (!AkEvent)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::PostAndWaitForEndOfEventAsync: No Event specified!"));
		PlayingID = AK_INVALID_PLAYING_ID;
		return;
	}

	if (!Actor)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::PostAndWaitForEndOfEventAsync: NULL Actor specified!"));
		PlayingID = AK_INVALID_PLAYING_ID;
		return;
	}

	AkDeviceAndWorld DeviceAndWorld(Actor);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		PlayingID  = AK_INVALID_PLAYING_ID;
	}

	FLatentActionManager& LatentActionManager = DeviceAndWorld.CurrentWorld->GetLatentActionManager();
	FWaitEndOfEventAsyncAction* NewAction = LatentActionManager.FindExistingAction<FWaitEndOfEventAsyncAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	if (!NewAction)
	{
		NewAction = new FWaitEndOfEventAsyncAction(LatentInfo, &PlayingID, AkEvent, bStopWhenAttachedToDestroyed);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		NewAction->FuturePlayingID = DeviceAndWorld.AkAudioDevice->PostAkAudioEventWithLatentActionOnActorAsync(AkEvent, Actor, bStopWhenAttachedToDestroyed, NewAction);
	}
}

void UAkGameplayStatics::PostEventByName(const FString& EventName, class AActor* Actor, bool bStopWhenAttachedToDestroyed)
{
	AkDeviceAndWorld DeviceAndWorld(Actor);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		return;
	}
	AkUInt32 ShortId = DeviceAndWorld.AkAudioDevice->GetShortID(nullptr, EventName);
	DeviceAndWorld.AkAudioDevice->PostEventOnActor(ShortId, Actor, 0, {}, {}, bStopWhenAttachedToDestroyed);
}

int32 UAkGameplayStatics::PostEventAtLocation(class UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation, const FString& EventName, UObject* WorldContextObject)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostAtLocation(Location, Orientation, {}, 0, WorldContextObject);
	}
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
		return AK_INVALID_PLAYING_ID;

	AkUInt32 ShortId = DeviceAndWorld.AkAudioDevice->GetShortID(nullptr, EventName);
	return DeviceAndWorld.AkAudioDevice->PostEventAtLocation(EventName, ShortId, Location, Orientation, DeviceAndWorld.CurrentWorld);
}

void UAkGameplayStatics::PostEventAtLocationByName(const FString& EventName, FVector Location, FRotator Orientation, UObject* WorldContextObject)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
		return;

	AkUInt32 ShortId = DeviceAndWorld.AkAudioDevice->GetShortID(nullptr, EventName);
	DeviceAndWorld.AkAudioDevice->PostEventAtLocation(EventName, ShortId, Location, Orientation, DeviceAndWorld.CurrentWorld);
}

UAkComponent* UAkGameplayStatics::SpawnAkComponentAtLocation(UObject* WorldContextObject, class UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation, bool AutoPost, const FString& EventName, bool AutoDestroy /* = true*/)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		return nullptr;
	}
	return DeviceAndWorld.AkAudioDevice->SpawnAkComponentAtLocation(AkEvent, Location, Orientation, AutoPost, EventName, AutoDestroy, DeviceAndWorld.CurrentWorld);
}

void UAkGameplayStatics::ExecuteActionOnEvent(class UAkAudioEvent* AkEvent, AkActionOnEventType ActionType, class AActor* Actor, int32 TransitionDuration, EAkCurveInterpolation FadeCurve, int32 PlayingID)
{
	if (!AkEvent)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ExecuteActionOnEvent: No Event specified!"));
		return;
	}

	AkEvent->ExecuteAction(ActionType, Actor, PlayingID, TransitionDuration, FadeCurve);
	
}

void UAkGameplayStatics::ExecuteActionOnPlayingID(AkActionOnEventType ActionType, int32 PlayingID, int32 TransitionDuration, EAkCurveInterpolation FadeCurve)
{
	if (PlayingID == AK_INVALID_PLAYING_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ExecuteActionOnPlayingID: Invalid Playing ID!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->ExecuteActionOnPlayingID(ActionType, PlayingID, TransitionDuration, FadeCurve);
	}
}

void UAkGameplayStatics::SetRTPCValue(const UAkRtpc* RTPCValue, float Value, int32 InterpolationTimeMs, AActor* Actor, FName RTPC)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		if (RTPCValue)
		{
			AudioDevice->SetRTPCValue(RTPCValue, Value, InterpolationTimeMs, Actor);
		}
		else if (RTPC.IsValid())
		{
			AudioDevice->SetRTPCValue(*RTPC.ToString(), Value, InterpolationTimeMs, Actor);
		}
	}
}

void UAkGameplayStatics::GetRTPCValue(const UAkRtpc* RTPCValue, int32 PlayingID, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType, AActor* Actor, FName RTPC)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AK::SoundEngine::Query::RTPCValue_type RTPCType = (AK::SoundEngine::Query::RTPCValue_type)InputValueType;

		AkGameObjectID IdToGet = AK_INVALID_GAME_OBJECT;
		if (Actor != nullptr)
		{
			UAkComponent * ComponentToGet = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
			IdToGet = ComponentToGet->GetAkGameObjectID();
		}

		if (RTPCValue)
		{
			AudioDevice->GetRTPCValue(RTPCValue, IdToGet, PlayingID, Value, RTPCType);
		}
		else if (RTPC.IsValid())
		{
			AudioDevice->GetRTPCValue(*RTPC.ToString(), IdToGet, PlayingID, Value, RTPCType);
		}

		OutputValueType = (ERTPCValueType)RTPCType;
	}
}

void UAkGameplayStatics::ResetRTPCValue(UAkRtpc const* RTPCValue, int32 InterpolationTimeMs, AActor* Actor, FName RTPC)
{

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AkGameObjectID IdToGet = AK_INVALID_GAME_OBJECT;
		if (Actor != nullptr)
		{
			UAkComponent* ComponentToGet = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
			IdToGet = ComponentToGet->GetAkGameObjectID();
		}

		if (RTPCValue == NULL && RTPC.IsNone())
		{
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ResetRTPCValue: No parameter specified!"));
			return;
		}


		AKRESULT Result = AK_Success;

		if (RTPCValue)
		{
			Result = AudioDevice->ResetRTPCValue(RTPCValue, IdToGet, InterpolationTimeMs);
		}
		else if (RTPC.IsValid())
		{
			Result = AudioDevice->ResetRTPCValue(*RTPC.ToString(), IdToGet, InterpolationTimeMs);
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ResetRTPCValue: Could not reset RTPC value, valid RTPC value not provided"));
		}

		if (Result == AK_IDNotFound)
		{
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ResetRTPCValue: Could not reset RTPC value, RTPC %s not found"), *RTPC.ToString());
		}
		else if (Result != AK_Success)
		{
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ResetRTPCValue: Could not reset RTPC value!"));
		}
	}
}

void UAkGameplayStatics::SetState(const UAkStateValue* StateValue, FName stateGroup, FName state)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice && stateGroup.IsValid() && state.IsValid() )
	{
		if (StateValue)
		{
			AudioDevice->SetState(StateValue);
		}
		else if (stateGroup.IsValid() && state.IsValid())
		{
			AudioDevice->SetState(*stateGroup.ToString(), *state.ToString());
		}
	}
}

void UAkGameplayStatics::PostTrigger(const UAkTrigger* TriggerValue, AActor* Actor, FName Trigger)
{
	if ( Actor == NULL )
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::PostTrigger: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice)
	{
		if (TriggerValue)
		{
			AudioDevice->PostTrigger(TriggerValue, Actor);
		}
		else if (Trigger.IsValid())
		{
			AudioDevice->PostTrigger(*Trigger.ToString(), Actor);
		}
	}
}

void UAkGameplayStatics::SetSwitch(const UAkSwitchValue* SwitchValue, AActor* Actor, FName SwitchGroup, FName SwitchState)
{
	if (Actor == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetSwitch: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		if (SwitchValue)
		{
			AudioDevice->SetSwitch(SwitchValue, Actor);
		}
		else if (SwitchGroup.IsValid() && SwitchState.IsValid())
		{
			AudioDevice->SetSwitch(*SwitchGroup.ToString(), *SwitchState.ToString(), Actor);
		}
	}
}

void UAkGameplayStatics::SetMultiplePositions(UAkComponent* GameObjectAkComponent, TArray<FTransform> Positions,
                                              AkMultiPositionType MultiPositionType /*= AkMultiPositionType::MultiPositionType_MultiDirections*/)
{
	if (GameObjectAkComponent == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetMultiplePositions: NULL Component specified!"));
		return;
	}

	FAkAudioDevice * pAudioDevice = FAkAudioDevice::Get();
    if (pAudioDevice)
    {
        pAudioDevice->SetMultiplePositions(GameObjectAkComponent, Positions, MultiPositionType);
    }
}

void UAkGameplayStatics::SetMultipleChannelEmitterPositions(UAkComponent* GameObjectAkComponent,
	TArray<AkChannelConfiguration> ChannelMasks,
	TArray<FTransform> Positions,
	AkMultiPositionType MultiPositionType
)
{
	if (GameObjectAkComponent == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetMultipleChannelEmitterPositions: NULL Component specified!"));
		return;
	}

	FAkAudioDevice * pAudioDevice = FAkAudioDevice::Get();
    if (pAudioDevice)
    {
        pAudioDevice->SetMultiplePositions(GameObjectAkComponent, ChannelMasks, Positions, MultiPositionType);
    }
}

void UAkGameplayStatics::SetMultipleChannelMaskEmitterPositions(UAkComponent* GameObjectAkComponent,
	TArray<FAkChannelMask> ChannelMasks,
	TArray<FTransform> Positions,
	AkMultiPositionType MultiPositionType
)
{
	if (GameObjectAkComponent == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetMultipleChannelMaskEmitterPositions: NULL Component specified!"));
		return;
	}

	FAkAudioDevice * pAudioDevice = FAkAudioDevice::Get();
	if (pAudioDevice)
	{
		pAudioDevice->SetMultiplePositions(GameObjectAkComponent, ChannelMasks, Positions, MultiPositionType);
	}
}

void UAkGameplayStatics::UseReverbVolumes(bool inUseReverbVolumes, class AActor* Actor )
{
	if ( Actor == NULL )
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::UseReverbVolumes: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice )
	{
		UAkComponent * ComponentToSet = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
		if( ComponentToSet != NULL )
		{
			ComponentToSet->UseReverbVolumes(inUseReverbVolumes);
		}
	}
}

void UAkGameplayStatics::UseEarlyReflections(class AActor* Actor,
	class UAkAuxBus* AuxBus,
	int Order,
	float BusSendGain,
	float MaxPathLength,
	bool SpotReflectors,
	const FString& AuxBusName)
{
	// Deprecated
}

void UAkGameplayStatics::SetReflectionsOrder(int Order, bool RefreshPaths)
{
	if (Order > 4 || Order < 0)
	{
		Order = FMath::Clamp(Order, 0, 4);
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetReflectionsOrder: The order value is invalid. It was clamped to %d"), Order);
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetReflectionsOrder(Order, RefreshPaths);
	}
}

void UAkGameplayStatics::SetPortalObstructionAndOcclusion(UAkPortalComponent* PortalComponent, float ObstructionValue, float OcclusionValue)
{
	if (ObstructionValue > 1.f || ObstructionValue < 0.f)
	{
		ObstructionValue = FMath::Clamp(ObstructionValue, 0.f, 1.f);
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetPortalObstructionAndOcclusion: The obstruction value is invalid. It was clamped to %f"), ObstructionValue);
	}

	if (OcclusionValue > 1.f || OcclusionValue < 0.f)
	{
		OcclusionValue = FMath::Clamp(OcclusionValue, 0.f, 1.f);
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetPortalObstructionAndOcclusion: The occlusion value is invalid. It was clamped to %f"), OcclusionValue);
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetPortalObstructionAndOcclusion(PortalComponent, ObstructionValue, OcclusionValue);
	}
}

void UAkGameplayStatics::SetGameObjectToPortalObstruction(UAkComponent* GameObjectAkComponent, UAkPortalComponent* PortalComponent, float ObstructionValue)
{
	if (ObstructionValue > 1.f || ObstructionValue < 0.f)
	{
		ObstructionValue = FMath::Clamp(ObstructionValue, 0.f, 1.f);
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetGameObjectToPortalObstruction: The obstruction value is invalid. It was clamped to %f"), ObstructionValue);
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetGameObjectToPortalObstruction(GameObjectAkComponent, PortalComponent, ObstructionValue);
	}
}

void UAkGameplayStatics::SetPortalToPortalObstruction(UAkPortalComponent* PortalComponent0, UAkPortalComponent* PortalComponent1, float ObstructionValue)
{
	if (ObstructionValue > 1.f || ObstructionValue < 0.f)
	{
		ObstructionValue = FMath::Clamp(ObstructionValue, 0.f, 1.f);
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetGameObjectToPortalObstruction: The obstruction value is invalid. It was clamped to %f"), ObstructionValue);
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetPortalToPortalObstruction(PortalComponent0, PortalComponent1, ObstructionValue);
	}
}

void UAkGameplayStatics::SetOutputBusVolume(float BusVolume, class AActor* Actor)
{
	if (Actor == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetOutputBusVolume: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		UAkComponent * ComponentToSet = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
		if (ComponentToSet != NULL)
		{
			ComponentToSet->SetOutputBusVolume(BusVolume);
		}
	}
}

void UAkGameplayStatics::SetBusConfig(const FString& BusName, AkChannelConfiguration ChannelConfiguration)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AkChannelConfig config;
	FAkAudioDevice::GetChannelConfig(ChannelConfiguration, config);
	AudioDevice->SetBusConfig(BusName, config);
}

void UAkGameplayStatics::SetPanningRule(PanningRule PanRule)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}

	AkPanningRule AkPanRule = (PanRule == PanningRule::PanningRule_Headphones) ? AkPanningRule_Headphones : AkPanningRule_Speakers;
	AudioDevice->SetPanningRule(AkPanRule);
}

void UAkGameplayStatics::ReplaceMainOutput(const FAkOutputSettings& MainOutputSettings)
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ReplaceMainOutput: Could not fetch audio device, main output will not be replaced."));
		return;
	}
	AkUInt32 ShortID = AudioDevice->GetShortIDFromString(MainOutputSettings.AudioDeviceShareSetName);
	if (UNLIKELY(ShortID == AK_INVALID_UNIQUE_ID))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::ReplaceMainOutput: Short ID for %s is invalid, main output will not be replaced."));
		return;
	}

	AkOutputSettings OutSettings;
	OutSettings.audioDeviceShareset = ShortID;
	OutSettings.idDevice = MainOutputSettings.IdDevice;
	OutSettings.ePanningRule = (MainOutputSettings.PanRule == PanningRule::PanningRule_Headphones) ? AkPanningRule_Headphones : AkPanningRule_Speakers;
	FAkAudioDevice::GetChannelConfig(MainOutputSettings.ChannelConfig, OutSettings.channelConfig);
	AudioDevice->ReplaceMainOutput(OutSettings);
}

void UAkGameplayStatics::GetSpeakerAngles(TArray<float>& SpeakerAngles, float& HeightAngle, const FString& DeviceShareSet)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AkOutputDeviceID DeviceID = DeviceShareSet.IsEmpty() ? 0 : AudioDevice->GetOutputID(DeviceShareSet);
	AudioDevice->GetSpeakerAngles(SpeakerAngles, HeightAngle, DeviceID);

}

void UAkGameplayStatics::SetSpeakerAngles(const TArray<float>& SpeakerAngles, float HeightAngles, const FString& DeviceShareSet)
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AkOutputDeviceID DeviceID = DeviceShareSet.IsEmpty() ? 0 : AudioDevice->GetOutputID(DeviceShareSet);
	AudioDevice->SetSpeakerAngles(SpeakerAngles, HeightAngles, DeviceID);

}

void UAkGameplayStatics::SetOcclusionRefreshInterval(float RefreshInterval, class AActor* Actor)
{
	if (Actor == NULL)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetOcclusionRefreshInterval: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetOcclusionRefreshInterval: Could not retrieve audio device."));
		return;
	}

	UAkComponent* ComponentToSet = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
	if (ComponentToSet != NULL)
	{
		ComponentToSet->OcclusionRefreshInterval = RefreshInterval;
	}
}
	

void UAkGameplayStatics::StopActor(class AActor* Actor)
{
	if ( Actor == NULL )
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::StopActor: NULL Actor specified!"));
		return;
	}

	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::StopActor: Could not retrieve audio device."));
		return;
	}

	AudioDevice->StopGameObject(AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset));
}

void UAkGameplayStatics::StopAll()
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}

	AudioDevice->StopAllSounds();
}

void UAkGameplayStatics::CancelEventCallback(const FOnAkPostEventCallback& PostEventCallback)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AudioDevice->CancelEventCallbackDelegate(PostEventCallback);
}

void UAkGameplayStatics::StartAllAmbientSounds(UObject* WorldContextObject)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		return;
	}

	for (FActorIterator It(DeviceAndWorld.CurrentWorld); It; ++It)
	{
		AAkAmbientSound* pAmbientSound = Cast<AAkAmbientSound>(*It);
		if (pAmbientSound != NULL)
		{
			UAkComponent* pComponent = pAmbientSound->AkComponent;
			if (pComponent && GWorld->Scene == pComponent->GetScene())
			{
				pAmbientSound->StartPlaying();
			}
		}
	}

}

void UAkGameplayStatics::StopAllAmbientSounds(UObject* WorldContextObject)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (UNLIKELY(!DeviceAndWorld.IsValid()))
	{
		return;
	}

	for (FActorIterator It(DeviceAndWorld.CurrentWorld); It; ++It)
	{
		AAkAmbientSound* pAmbientSound = Cast<AAkAmbientSound>(*It);
		if (pAmbientSound != NULL)
		{
			UAkComponent* pComponent = pAmbientSound->AkComponent;
			if (pComponent && GWorld->Scene == pComponent->GetScene())
			{
				pAmbientSound->StopPlaying();
			}
		}
	}
}

void UAkGameplayStatics::ClearSoundBanksAndMedia()
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AudioDevice->ClearSoundBanksAndMedia();
}

void UAkGameplayStatics::ClearBanks()
{
	ClearSoundBanksAndMedia();
}

void UAkGameplayStatics::LoadInitBank()
{
	auto* InitBankLoader = FWwiseInitBankLoader::Get();
	if(UNLIKELY(!InitBankLoader))
	{
		UE_LOG(LogAkAudio, Error, TEXT("LoadInitBank: WwiseInitBankLoader is not initialized."));
		return;
	}

	InitBankLoader->LoadInitBank();
}

void UAkGameplayStatics::UnloadInitBank()
{
	auto* InitBankLoader = FWwiseInitBankLoader::Get();
	if(UNLIKELY(!InitBankLoader))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UnloadInitBank: WwiseInitBankLoader is not initialized."));
		return;
	}

	InitBankLoader->UnloadInitBank();
}

void UAkGameplayStatics::LoadBankByName(const FString& BankName)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AkBankID BankId;
	AudioDevice->LoadBank(BankName, BankId);
}


void UAkGameplayStatics::UnloadBankByName(const FString& BankName)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	AudioDevice->UnloadBank(BankName);
}

void UAkGameplayStatics::StartOutputCapture(const FString& Filename)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return;
	}
	FString name = Filename;
	if (!name.EndsWith(".wav"))
	{
		name += ".wav";
	}
	AudioDevice->StartOutputCapture(name);
}

void UAkGameplayStatics::AddOutputCaptureMarker(const FString& MarkerText)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->AddOutputCaptureMarker(MarkerText);
	} 
}

void UAkGameplayStatics::StopOutputCapture()
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->StopOutputCapture();
	}
}

void UAkGameplayStatics::StartProfilerCapture(const FString& Filename)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice )
	{
		FString name = Filename;
		if( !name.EndsWith(".prof") )
		{
			name += ".prof";
		}
		AudioDevice->StartProfilerCapture(name);
	} 
}

void UAkGameplayStatics::StopProfilerCapture()
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->StopProfilerCapture();
	}
}

FString UAkGameplayStatics::GetCurrentAudioCulture()
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		return AudioDevice->GetCurrentAudioCulture();
	}

	return FString();
}

TArray<FString> UAkGameplayStatics::GetAvailableAudioCultures()
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		return AudioDevice->GetAvailableAudioCultures();
	}

	return TArray<FString>();
}

void UAkGameplayStatics::SetCurrentAudioCulture(const FString& AudioCulture, FLatentActionInfo LatentInfo, UObject* WorldContextObject)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	FLatentActionManager& LatentActionManager = DeviceAndWorld.CurrentWorld->GetLatentActionManager();
	FSetCurrentAudioCultureAction* NewAction = LatentActionManager.FindExistingAction<FSetCurrentAudioCultureAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	if (!NewAction)
	{
		NewAction = new FSetCurrentAudioCultureAction(LatentInfo);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->SetCurrentAudioCultureAsync(AudioCulture, NewAction);
	}
	else
	{
		NewAction->ActionDone = true;
	}
}

void UAkGameplayStatics::SetCurrentAudioCultureAsync(const FString& AudioCulture, const FOnSetCurrentAudioCultureCallback& Completed)
{
	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->SetCurrentAudioCultureAsync(AudioCulture, FOnSetCurrentAudioCultureCompleted::CreateLambda([Completed](bool Succeeded) {
			Completed.ExecuteIfBound(Succeeded);
		}));
	}
}

UObject* UAkGameplayStatics::GetAkAudioTypeUserData(const UAkAudioType* Instance, const UClass* Type)
{
	for (auto entry : Instance->UserData)
	{
		if (entry && entry->GetClass()->IsChildOf(Type))
		{
			return entry;
		}
	}

	return nullptr;
}

void UAkGameplayStatics::SetDistanceProbe(AActor* Listener, AActor* DistanceProbe)
{
	if (Listener == nullptr)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetDistanceProbe: NULL Listener specified!"));
		return;
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		UAkComponent * ListenerAkComponent = AudioDevice->GetAkComponent(Listener->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);

		UAkComponent* DistanceProbeAkComponent = 
			DistanceProbe != nullptr ?
			AudioDevice->GetAkComponent(DistanceProbe->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset) :
			nullptr;

		AudioDevice->SetDistanceProbe(ListenerAkComponent, DistanceProbeAkComponent);
	}
}

bool UAkGameplayStatics::SetOutputDeviceEffect(const FAkOutputDeviceID InDeviceID, const int32 InEffectIndex, const UAkEffectShareSet* InEffectShareSet)
{
	if(UNLIKELY(!InEffectShareSet)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetOutputDeviceEffect: NULL Effect ShareSet specified!"));
		return false;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return false;

	UE_LOG(LogAkAudio, Verbose, TEXT("UAkGameplayStatics::SetOutputDeviceEffect: DeviceID: %" PRIu64 ", InEffectIndex: %d, EffectShareSet Asset Name: %s, EffectShareSet ShortID: %" PRIu32 "."), 
		InDeviceID.UInt64Value, InEffectIndex, *InEffectShareSet->GetName(), InEffectShareSet->GetShortID());

	AKRESULT Result =  SoundEngine->SetOutputDeviceEffect(InDeviceID.UInt64Value, InEffectIndex, InEffectShareSet->GetShortID());
	return Result == AK_Success;
}

bool UAkGameplayStatics::SetBusEffectByName(const FString InBusName, const int32 InEffectIndex, const UAkEffectShareSet* InEffectShareSet)
{
	if(UNLIKELY(!InEffectShareSet)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetBusEffectByName: NULL Effect ShareSet specified!"));
		return false;
	}
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return false;

	UE_LOG(LogAkAudio, Verbose, TEXT("UAkGameplayStatics::SetBusEffectByName: BusName: %s, InEffectIndex: %d,  EffectShareSet Asset Name: %s, EffectShareSet ShortID: %" PRIu32 "."), 
		*InBusName, InEffectIndex, *InEffectShareSet->GetName(), InEffectShareSet->GetShortID());

	AKRESULT Result = SoundEngine->SetBusEffect(TCHAR_TO_AK(*InBusName), InEffectIndex, InEffectShareSet->GetShortID());
	return Result == AK_Success;
}

bool UAkGameplayStatics::SetBusEffectByID(const FAkUniqueID InBusID, const int32 InEffectIndex, const UAkEffectShareSet* InEffectShareSet)
{
	if(UNLIKELY(!InEffectShareSet)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetBusEffectByID: NULL Effect ShareSet specified!"));
		return false;
	}
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return false;

	UE_LOG(LogAkAudio, Verbose, TEXT("UAkGameplayStatics::SetBusEffectByID: BusID: %" PRIu32 ", InEffectIndex: %d, EffectShareSet Asset Name: %s, EffectShareSet ShortID: %" PRIu32 "."), 
		InBusID.UInt32Value, InEffectIndex, *InEffectShareSet->GetName(), InEffectShareSet->GetShortID());
	AKRESULT Result = SoundEngine->SetBusEffect(InBusID.UInt32Value, InEffectIndex, InEffectShareSet->GetShortID());
	return Result == AK_Success;
}

bool UAkGameplayStatics::SetAuxBusEffect(const UAkAuxBus* InAuxBus, const int32 InEffectIndex, const UAkEffectShareSet* InEffectShareSet)
{
	if(UNLIKELY(!InEffectShareSet)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetAuxBusEffect: NULL Effect ShareSet specified!"));
		return false;
	}
	if(UNLIKELY(!InAuxBus)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetAuxBusEffect: NULL Aux Bus specified!"));
		return false;
	}
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return false;

	UE_LOG(LogAkAudio, Verbose, TEXT("UAkGameplayStatics::SetAuxBusEffect: AuxBus Asset Name: %s, AuxBus Short ID: %" PRIu32 ", InEffectIndex: %d, EffectShareSet Asset Name: %s, EffectShareSet ShortID: %" PRIu32 "."), 
		*InAuxBus->GetName(), InAuxBus->GetShortID(), InEffectIndex, *InEffectShareSet->GetName(), InEffectShareSet->GetShortID());
	AKRESULT Result = SoundEngine->SetBusEffect(InAuxBus->GetShortID(), InEffectIndex, InEffectShareSet->GetShortID());
	return Result == AK_Success;
}

bool UAkGameplayStatics::SetActorMixerEffect(const FAkUniqueID InAudioNodeID,const  int32 InEffectIndex, const UAkEffectShareSet* InEffectShareSet)
{
	if(UNLIKELY(!InEffectShareSet)) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkGameplayStatics::SetActorMixerEffect: NULL Effect ShareSet specified!"));
		return false;
	}
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return false;

	UE_LOG(LogAkAudio, Verbose, TEXT("UAkGameplayStatics::SetActorMixerEffect: AudioNodeID: %" PRIu32 ", InEffectIndex: %d, EffectShareSet Asset Name: %s, EffectShareSet ShortID: %" PRIu32 "."), 
		InAudioNodeID.UInt32Value, InEffectIndex, *InEffectShareSet->GetName(), InEffectShareSet->GetShortID());
	AKRESULT Result = SoundEngine->SetActorMixerEffect(InAudioNodeID.UInt32Value, InEffectIndex, InEffectShareSet->GetShortID());
	return Result == AK_Success;
}
