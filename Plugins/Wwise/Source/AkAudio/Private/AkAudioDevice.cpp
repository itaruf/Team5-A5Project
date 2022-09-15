/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/


/*=============================================================================
	AkAudioDevice.cpp: Audiokinetic Audio interface object.
=============================================================================*/

#define AK_ENABLE_ROOMS
#define AK_ENABLE_PORTALS

#include "AkAudioDevice.h"

#include "AkAcousticPortal.h"
#include "AkAudioEvent.h"
#include "AkAudioModule.h"
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseIOHook.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseResourceLoaderImpl.h"
#include "Wwise/LowLevel/WwiseLowLevelComm.h"
#include "Wwise/LowLevel/WwiseLowLevelMonitor.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/LowLevel/WwiseLowLevelSpatialAudio.h"
#include "Wwise/LowLevel/WwiseLowLevelStreamMgr.h"
#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#include "AkCallbackInfoPool.h"
#include "AkComponent.h"
#include "AkComponentCallbackManager.h"
#include "AkComponentHelpers.h"
#include "AkGameObject.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkRtpc.h"
#include "AkSettings.h"

#include "AkSpotReflector.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkUnrealHelper.h"
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"

#include "EngineUtils.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/BrushComponent.h"
#include "Engine/GameEngine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "InitializationSettings/AkInitializationSettings.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/App.h"
#include "Misc/ScopeLock.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Stats/Stats.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include "Wwise/WwiseExternalSourceManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EditorSupportDelegates.h"
#include "LevelEditor.h"
#include "UnrealEdMisc.h"

#ifndef AK_OPTIMIZED
#include "AkSettingsPerUser.h"
#endif

#endif

DEFINE_LOG_CATEGORY(LogAkAudio);

/*------------------------------------------------------------------------------------
	Statics and Globals
------------------------------------------------------------------------------------*/

bool FAkAudioDevice::m_bSoundEngineInitialized = false;
bool FAkAudioDevice::m_EngineExiting = false;
TMap<uint32, TArray<uint32>> FAkAudioDevice::EventToPlayingIDMap;
TMap<uint32, FOnSwitchValueLoaded> FAkAudioDevice::OnSwitchValueLoadedMap;

FCriticalSection FAkAudioDevice::EventToPlayingIDMapCriticalSection;

/*------------------------------------------------------------------------------------
	Defines
------------------------------------------------------------------------------------*/

#define GAME_OBJECT_MAX_STRING_SIZE 512
#define AK_READ_SIZE DVD_MIN_READ_SIZE

static constexpr auto InvariantLCID = 0x7F;

DECLARE_STATS_GROUP(TEXT("AkAudioDevice"), STATGROUP_AkAudioDevice, STATCAT_Wwise);
DECLARE_CYCLE_STAT(TEXT("Post Event Async"), STAT_PostEventAsync, STATGROUP_AkAudioDevice);

/*------------------------------------------------------------------------------------
	Helpers
------------------------------------------------------------------------------------*/

namespace FAkAudioDevice_Helpers
{
	void RegisterGameObject(AkGameObjectID in_gameObjId, const FString& Name)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

#ifdef AK_OPTIMIZED
		SoundEngine->RegisterGameObj(in_gameObjId);
#else
		if (Name.Len() > 0)
		{
			SoundEngine->RegisterGameObj(in_gameObjId, TCHAR_TO_ANSI(*Name));
		}
		else
		{
			SoundEngine->RegisterGameObj(in_gameObjId);
		}
#endif
	}

	typedef TMap<AkGlobalCallbackLocation, FAkAudioDeviceDelegates::FOnAkGlobalCallback> FDelegateLocationMap;
	FDelegateLocationMap DelegateLocationMap;

	void GlobalCallback(AK::IAkGlobalPluginContext* Context, AkGlobalCallbackLocation Location, void* Cookie)
	{
		const FAkAudioDeviceDelegates::FOnAkGlobalCallback* Delegate = DelegateLocationMap.Find(Location);
		if (Delegate && Delegate->IsBound())
		{
			Delegate->Broadcast(Context, Location);
		}
	}

	void UnregisterGlobalCallbackDelegate(FAkAudioDeviceDelegates::FOnAkGlobalCallback* Delegate, FDelegateHandle Handle, AkGlobalCallbackLocation Location)
	{
		if (!Delegate)
			return;

		Delegate->Remove(Handle);
		if (Delegate->IsBound())
			return;

		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->UnregisterGlobalCallback(GlobalCallback, Location);
	}

	void UnregisterAllGlobalCallbacks()
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();

		if (LIKELY(SoundEngine))
		{
			for (auto DelegateLocationPair : FAkAudioDevice_Helpers::DelegateLocationMap)
			{
				auto Location = DelegateLocationPair.Key;
				SoundEngine->UnregisterGlobalCallback(GlobalCallback, Location);
			}
		}
		FAkAudioDevice_Helpers::DelegateLocationMap.Empty();
	}
}

#if WITH_EDITOR
class AkUnrealErrorTranslator : public AkErrorMessageTranslator
{
	virtual void Term() override {};

protected:
	virtual bool GetInfo(TagInformation* in_pTagList, AkUInt32 in_uCount, AkUInt32& out_uTranslated) override
	{
#ifndef AK_OPTIMIZED
		for (AkUInt32 i = 0; i < in_uCount && out_uTranslated != in_uCount; i++)
		{
			if (!in_pTagList[i].m_infoIsParsed && *in_pTagList[i].m_pTag == 'g')
			{
				AkUInt64 gId = FPlatformMemory::ReadUnaligned<AkUInt64>((AkUInt8*)in_pTagList[i].m_args);
				
				if (gId == AK_INVALID_GAME_OBJECT)
					continue;

				UAkComponent* pComponent = UAkComponent::GetAkComponent(gId);
				
				if (IsValid(pComponent))
				{
					FString WwiseGameObjectName;
					pComponent->GetAkGameObjectName(WwiseGameObjectName);
					in_pTagList[i].m_len = WwiseGameObjectName.Len();
					AKPLATFORM::SafeStrCpy(in_pTagList[i].m_parsedInfo, TCHAR_TO_AK(*WwiseGameObjectName), AK_TRANSLATOR_MAX_NAME_SIZE);
					in_pTagList[i].m_infoIsParsed = true;
					out_uTranslated++;
				}
			}
		}
		return out_uTranslated == in_uCount;
#endif
	}
};
#endif

/*------------------------------------------------------------------------------------
	Implementation
------------------------------------------------------------------------------------*/

FDelegateHandle FAkAudioDevice::RegisterGlobalCallback(FAkAudioDeviceDelegates::FOnAkGlobalCallback::FDelegate Callback, AkGlobalCallbackLocation Location)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return {};

	auto& Delegate = FAkAudioDevice_Helpers::DelegateLocationMap.FindOrAdd(Location);
	FDelegateHandle Handle = Delegate.Add(Callback);
	auto result = SoundEngine->RegisterGlobalCallback(FAkAudioDevice_Helpers::GlobalCallback, Location);
	if (result != AK_Success)
	{
		FAkAudioDevice_Helpers::UnregisterGlobalCallbackDelegate(&Delegate, Handle, Location);
		Handle.Reset();
	}

	return Handle;
}

void FAkAudioDevice::UnregisterGlobalCallback(FDelegateHandle Handle, AkGlobalCallbackLocation Location)
{
	const auto& Delegate = FAkAudioDevice_Helpers::DelegateLocationMap.Find(Location);
	FAkAudioDevice_Helpers::UnregisterGlobalCallbackDelegate(Delegate, Handle, Location);
}

AKRESULT FAkAudioDevice::RegisterOutputDeviceMeteringCallback(AkOutputDeviceID OutputID,
															  AkOutputDeviceMeteringCallbackFunc Callback,
															  AkMeteringFlags MeteringFlags,
															  void* Cookie)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->RegisterOutputDeviceMeteringCallback(OutputID, Callback, MeteringFlags, Cookie);
}

AKRESULT FAkAudioDevice::UnregisterOutputDeviceMeteringCallback(AkOutputDeviceID OutputID)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->RegisterOutputDeviceMeteringCallback(OutputID, nullptr, AK_NoMetering, nullptr);
}

#if WITH_EDITORONLY_DATA
UAkComponent* FAkAudioDevice::CreateListener(UWorld* World, FEditorViewportClient* ViewportClient)
{
	if (!IsRunningGame())
	{
		FString ComponentName = TEXT("AkListener_") + World->GetName();
		if (ViewportClient)
		{
			ComponentName = TEXT("AkListener_") + FString::FromInt(ViewportClient->ViewIndex) + World->GetName();
		}
		UAkComponent* Listener = NewObject<UAkComponent>(World->GetWorldSettings(), FName(*ComponentName), RF_Transient);
		if (Listener != nullptr)
		{
			Listener->MarkAsEditorOnlySubobject();
			Listener->RegisterComponentWithWorld(World);
			AddDefaultListener(Listener);
		}

		return Listener;
	}
	else
	{
		return nullptr;
	}
}

FTransform FAkAudioDevice::GetEditorListenerPosition(int32 ViewIndex) const
{
	if (ViewIndex < ListenerTransforms.Num())
	{
		return ListenerTransforms[ViewIndex];
	}

	return FTransform();
}

#endif

bool FAkAudioDevice::ShouldNotifySoundEngine(EWorldType::Type WorldType) { return WorldType == EWorldType::PIE || WorldType == EWorldType::Game; }

namespace FAkAudioDevice_WaapiHelper
{
	void Subscribe(FAkWaapiClient* waapiClient, uint64& subscriptionId, const char* uri, const TSharedRef<FJsonObject>& options, WampEventCallback callback)
	{
		if (subscriptionId == 0)
		{
			TSharedPtr<FJsonObject> result;
			waapiClient->Subscribe(uri, options, callback, subscriptionId, result);
		}
	}

	void RemoveCallback(FAkWaapiClient* waapiClient, uint64& subscriptionId)
	{
		if (subscriptionId > 0)
		{
			waapiClient->RemoveWampEventCallback(subscriptionId);
			subscriptionId = 0;
		}
	}

	void Unsubscribe(FAkWaapiClient* waapiClient, uint64& subscriptionId)
	{
		if (subscriptionId > 0)
		{
			TSharedPtr<FJsonObject> result;
			waapiClient->Unsubscribe(subscriptionId, result);

			RemoveCallback(waapiClient, subscriptionId);
		}
	}
}

/**
 * Initializes the audio device and creates sources.
 *
 * @return true if initialization was successful, false otherwise
 */
bool FAkAudioDevice::Init()
{
#if UE_SERVER
	return false;
#endif
	if (!EnsureInitialized()) // ensure audiolib is initialized
	{
		UE_LOG(LogAkAudio, Log, TEXT("Audiokinetic Audio Device initialization failed."));
		return false;
	}

#if !WITH_EDITOR
	if (auto* akSettings = GetDefault<UAkSettings>())
	{
		for (auto& entry : akSettings->UnrealCultureToWwiseCulture)
		{
			auto culturePtr = FInternationalization::Get().GetCulture(entry.Key);
			if (culturePtr && culturePtr->GetLCID() != InvariantLCID)
			{
				CachedUnrealToWwiseCulture.Add(culturePtr, entry.Value);
			}
		}
	}
#endif

	

#if AK_SUPPORT_WAAPI
	if (auto waapiClient = FAkWaapiClient::Get())
	{
		ProjectLoadedHandle = waapiClient->OnProjectLoaded.AddLambda([this, waapiClient]
		{
			if (!waapiClient->IsConnected())
				return;

			TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
			options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>> { MakeShareable(new FJsonValueString(WwiseWaapiHelper::PARENT)) });

			auto wampEventCallback = WampEventCallback::CreateLambda([this](uint64_t id, TSharedPtr<FJsonObject> in_UEJsonObject)
			{
				AsyncTask(ENamedThreads::GameThread, [this]
				{
#if WITH_EDITOR
					FEditorSupportDelegates::RedrawAllViewports.Broadcast();
#endif
					OnWwiseProjectModification.Broadcast();
				});
			});

			FAkAudioDevice_WaapiHelper::Subscribe(waapiClient, WaapiSubscriptionIds.Renamed, ak::wwise::core::object::nameChanged, options, wampEventCallback);
			FAkAudioDevice_WaapiHelper::Subscribe(waapiClient, WaapiSubscriptionIds.PreDeleted, ak::wwise::core::object::preDeleted, options, wampEventCallback);
			FAkAudioDevice_WaapiHelper::Subscribe(waapiClient, WaapiSubscriptionIds.ChildRemoved, ak::wwise::core::object::childRemoved, options, wampEventCallback);
			FAkAudioDevice_WaapiHelper::Subscribe(waapiClient, WaapiSubscriptionIds.ChildAdded, ak::wwise::core::object::childAdded, options, wampEventCallback);
			FAkAudioDevice_WaapiHelper::Subscribe(waapiClient, WaapiSubscriptionIds.Created, ak::wwise::core::object::created, options, wampEventCallback);
		});

		ConnectionLostHandle = waapiClient->OnConnectionLost.AddLambda([this, waapiClient]
		{
			if (!waapiClient->IsConnected())
				return;

			FAkAudioDevice_WaapiHelper::RemoveCallback(waapiClient, WaapiSubscriptionIds.Renamed);
			FAkAudioDevice_WaapiHelper::RemoveCallback(waapiClient, WaapiSubscriptionIds.PreDeleted);
			FAkAudioDevice_WaapiHelper::RemoveCallback(waapiClient, WaapiSubscriptionIds.ChildRemoved);
			FAkAudioDevice_WaapiHelper::RemoveCallback(waapiClient, WaapiSubscriptionIds.ChildAdded);
			FAkAudioDevice_WaapiHelper::RemoveCallback(waapiClient, WaapiSubscriptionIds.Created);
		});

		ClientBeginDestroyHandle = waapiClient->OnClientBeginDestroy.AddLambda([this, waapiClient]
		{
			if (ProjectLoadedHandle.IsValid())
			{
				waapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
				ProjectLoadedHandle.Reset();
			}

			if (ConnectionLostHandle.IsValid())
			{
				waapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
				ConnectionLostHandle.Reset();
			}
		});
	}
#endif // AK_SUPPORT_WAAPI

	FWorldDelegates::OnPreWorldInitialization.AddLambda(
		[&](UWorld* World, const UWorld::InitializationValues IVS)
		{
			WorldVolumesUpdatedMap.Add(World, false);
		}
	);

	FWorldDelegates::OnPostWorldInitialization.AddLambda(
		[&](UWorld* World, const UWorld::InitializationValues IVS)
		{
			World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &FAkAudioDevice::OnActorSpawned));
		}
	);
	
	FWorldDelegates::OnWorldCleanup.AddLambda(
		[this](UWorld* World, bool bSessionEnded, bool bCleanupResources)
		{
			CleanupComponentMapsForWorld(World);
			if (WorldVolumesUpdatedMap.Contains(World))
				WorldVolumesUpdatedMap.Remove(World);
		}
	);

	FWorldDelegates::OnWorldPostActorTick.AddLambda(
		[&](UWorld* World, ELevelTick TickType, float DeltaSeconds)
		{
			if (WorldVolumesUpdatedMap.Contains(World))
				WorldVolumesUpdatedMap[World] = false;
			else
				WorldVolumesUpdatedMap.Add(World, false);
		}
	);

#if WITH_EDITOR
	FEditorSupportDelegates::PrepareToCleanseEditorObject.AddLambda
	(
		[this](UObject* Object)
		{
			auto Level = Cast<ULevel>(Object);
			if (Level != nullptr)
			{
			}
	});
#endif// WITH_EDITOR

	m_SpatialAudioListener = nullptr;

#if WITH_EDITORONLY_DATA
	if (!IsRunningGame())
	{
		static const FName kLevelEditorModuleName = TEXT("LevelEditor");

		auto MapChangedHandler = [this](UWorld* World, EMapChangeType MapChangeType)
		{
			if (World && World->AllowAudioPlayback() && World->WorldType == EWorldType::Editor)
			{
				if (MapChangeType == EMapChangeType::TearDownWorld)
				{
					if (EditorListener && World == EditorListener->GetWorld())
					{
						EditorListener->DestroyComponent();
						EditorListener = nullptr;
					}
				}
				else if (EditorListener == nullptr && MapChangeType != EMapChangeType::SaveMap)
				{
					EditorListener = CreateListener(World);

					// The Editor Listener should NEVER be the spatial audio listener
					if (m_SpatialAudioListener == EditorListener)
					{
						auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
						if (LIKELY(SpatialAudio))
						{
							SpatialAudio->UnregisterListener(m_SpatialAudioListener->GetAkGameObjectID());
						}
						m_SpatialAudioListener = nullptr;
					}
				}
			}
		};

		auto LevelEditorModulePtr = FModuleManager::Get().GetModulePtr<FLevelEditorModule>(kLevelEditorModuleName);
		if (LevelEditorModulePtr)
		{
			LevelEditorModulePtr->OnMapChanged().AddLambda(MapChangedHandler);
		}
		else
		{
			FModuleManager::Get().OnModulesChanged().AddLambda([this, MapChangedHandler](FName InModuleName, EModuleChangeReason Reason)
			{
				if (InModuleName == kLevelEditorModuleName && Reason == EModuleChangeReason::ModuleLoaded)
				{
					auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(kLevelEditorModuleName);
					LevelEditorModule.OnMapChanged().AddLambda(MapChangedHandler);
				}
			});
		}

		FEditorDelegates::OnEditorCameraMoved.AddLambda(
			[&](const FVector& Location, const FRotator& Rotation, ELevelViewportType ViewportType, int32 ViewIndex)
			{
				auto& allViewportClient = GEditor->GetAllViewportClients();
				if (allViewportClient[ViewIndex]->Viewport && allViewportClient[ViewIndex]->Viewport->HasFocus())
				{
					if (ListenerTransforms.Num() <= ViewIndex)
					{
						ListenerTransforms.AddDefaulted(ViewIndex - ListenerTransforms.Num() + 1);
					}
					ListenerTransforms[ViewIndex].SetLocation(Location);
					ListenerTransforms[ViewIndex].SetRotation(Rotation.Quaternion());

					UWorld * ViewportWorld = allViewportClient[ViewIndex]->GetWorld();
					if (ViewportWorld && ViewportWorld->WorldType != EWorldType::PIE)
					{
						auto Quat = Rotation.Quaternion();
						AkSoundPosition soundpos;
						FVectorsToAKWorldTransform(
							Location,
							Quat.GetForwardVector(),
							Quat.GetUpVector(),
							soundpos
						);

						SetPosition(EditorListener, soundpos);
					}
				}
			}
		);

		FEditorDelegates::BeginPIE.AddRaw(this, &FAkAudioDevice::BeginPIE);
		FEditorDelegates::EndPIE.AddRaw(this, &FAkAudioDevice::EndPIE);
		FEditorDelegates::OnSwitchBeginPIEAndSIE.AddRaw(this, &FAkAudioDevice::OnSwitchBeginPIEAndSIE);
	}
#endif
	UE_LOG(LogAkAudio, Log, TEXT("Audiokinetic Audio Device initialized."));

	return 1;
}

#if WITH_EDITORONLY_DATA

void FAkAudioDevice::BeginPIE(const bool bIsSimulating)
{
	if (!bIsSimulating)
	{
		RemoveDefaultListener(EditorListener);
	}
}

void FAkAudioDevice::OnSwitchBeginPIEAndSIE(const bool bIsSimulating)
{
	if (bIsSimulating)
	{
		AddDefaultListener(EditorListener);
		// The Editor Listener should NEVER be the spatial audio listener
		if (m_SpatialAudioListener == EditorListener)
		{
			auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
			if (LIKELY(SpatialAudio))
			{
				SpatialAudio->UnregisterListener(m_SpatialAudioListener->GetAkGameObjectID());
			}
			m_SpatialAudioListener = nullptr;
		}
	}
	else
	{
		RemoveDefaultListener(EditorListener);
	}
}

void FAkAudioDevice::EndPIE(const bool bIsSimulating)
{
	//Reset Unreal Global gameobject to avoid complications from removing Spatial Audio listener
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (LIKELY(SoundEngine))
	{
		AkGameObjectID tempID = DUMMY_GAMEOBJ;
		SoundEngine->SetListeners(DUMMY_GAMEOBJ, &tempID, 1);
	}

	if (!bIsSimulating)
	{
		AddDefaultListener(EditorListener);

		// The Editor Listener should NEVER be the spatial audio listener
		if (m_SpatialAudioListener == EditorListener)
		{
			auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
			if (LIKELY(SpatialAudio))
			{
				SpatialAudio->UnregisterListener(m_SpatialAudioListener->GetAkGameObjectID());
			}
			m_SpatialAudioListener = nullptr;
		}
	}
	StopAllSounds();
}
#endif
void FAkAudioDevice::UpdateRoomsForPortals(UWorld* World)
{
#ifdef AK_ENABLE_ROOMS
	auto Portals = WorldPortalsMap.Find(World);
	if (Portals != nullptr)
	{
		for (auto Portal : *Portals)
		{
			const bool RoomsChanged = Portal->UpdateConnectedRooms();
			if (RoomsChanged)
				SetSpatialAudioPortal(Portal);
		}
	}
#endif
}

void FAkAudioDevice::CleanupComponentMapsForWorld(UWorld* World)
{
	LateReverbIndex.Clear(World);
	RoomIndex.Clear(World);
	WorldPortalsMap.Remove(World);
}

/**
 * Update the audio device and calculates the cached inverse transform later
 * on used for spatialization.
 */
bool FAkAudioDevice::Update( float DeltaTime )
{
	if (m_bSoundEngineInitialized)
	{
		// Suspend audio when not in VR focus
		if (FApp::UseVRFocus())
		{
			if (FApp::HasVRFocus())
			{
				WakeupFromSuspend();
			}
			else
			{
				Suspend(true);
			}
		}

		UpdateSetCurrentAudioCultureAsyncTasks();

		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return false;

		SoundEngine->RenderAudio();
	}

	return true;
}

/**
 * Tears down audio device by stopping all sounds, removing all buffers, 
 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
 * to perform the actual tear down.
 */
void FAkAudioDevice::Teardown()
{
#if AK_SUPPORT_WAAPI
	if (auto waapiClient = FAkWaapiClient::Get())
	{
		FAkAudioDevice_WaapiHelper::Unsubscribe(waapiClient, WaapiSubscriptionIds.Renamed);
		FAkAudioDevice_WaapiHelper::Unsubscribe(waapiClient, WaapiSubscriptionIds.PreDeleted);
		FAkAudioDevice_WaapiHelper::Unsubscribe(waapiClient, WaapiSubscriptionIds.ChildRemoved);
		FAkAudioDevice_WaapiHelper::Unsubscribe(waapiClient, WaapiSubscriptionIds.ChildAdded);
		FAkAudioDevice_WaapiHelper::Unsubscribe(waapiClient, WaapiSubscriptionIds.Created);

		if (ProjectLoadedHandle.IsValid())
		{
			waapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
			ProjectLoadedHandle.Reset();
		}

		if (ConnectionLostHandle.IsValid())
		{
			waapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
			ConnectionLostHandle.Reset();
		}

		if (ClientBeginDestroyHandle.IsValid())
		{
			waapiClient->OnClientBeginDestroy.Remove(ClientBeginDestroyHandle);
			ClientBeginDestroyHandle.Reset();
		}

		OnWwiseProjectModification.Clear();
	}
#endif // AK_SUPPORT_WAAPI

	if (m_bSoundEngineInitialized)
	{
		InitBank = nullptr;

		m_EngineExiting = true;

		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (LIKELY(SoundEngine))
		{
			SoundEngine->UnregisterGameObj(DUMMY_GAMEOBJ);

			if (SoundEngine->IsInitialized())
			{
				FAkAudioDevice_Helpers::UnregisterAllGlobalCallbacks();

				SoundEngine->StopAll();
				SoundEngine->RenderAudio();
			}
		}

		FAkSoundEngineInitialization::Finalize(IOHook);

		if (LIKELY(CallbackManager))
		{
			delete CallbackManager;
			CallbackManager = nullptr;
		}

		if (LIKELY(CallbackInfoPool))
		{
			delete CallbackInfoPool;
			CallbackInfoPool = nullptr;
		}

		if (LIKELY(IOHook))
		{
			delete IOHook;
			IOHook = nullptr;
		}

		auto* Monitor = FWwiseLowLevelMonitor::Get();
		if (LIKELY(Monitor))
		{
			//Monitor->TerminateDefaultXMLErrorTranslator(); TODO: see WG-60849
			Monitor->TerminateDefaultWAAPIErrorTranslator();
			Monitor->ResetTranslator();
		}

		m_bSoundEngineInitialized = false;
	}

	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);

#if !WITH_EDITOR
	CachedUnrealToWwiseCulture.Empty();
#endif

	UE_LOG(LogAkAudio, Log, TEXT("Audiokinetic Audio Device terminated."));
}

/**
 * Stops all game sounds (and possibly UI) sounds
 *
 * @param bShouldStopUISounds If true, this function will stop UI sounds as well
 */
void FAkAudioDevice::StopAllSounds(bool bShouldStopUISounds)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	SoundEngine->StopAll(DUMMY_GAMEOBJ);
	SoundEngine->StopAll();
}

/**
 * Stop all audio associated with a scene
 *
 * @param SceneToFlush		Interface of the scene to flush
 */
void FAkAudioDevice::Flush(UWorld* WorldToFlush)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	SoundEngine->StopAll(DUMMY_GAMEOBJ);
	SoundEngine->StopAll();
}

/**
* Determine if any rooms or reverb volumes have been added to World during the current frame
*/
bool FAkAudioDevice::WorldSpatialAudioVolumesUpdated(UWorld* World)
{
	if (World == nullptr)
		return false;

	if (WorldVolumesUpdatedMap.Contains(World))
		return WorldVolumesUpdatedMap[World];

	return false;
}

/**
 * Clears all loaded soundbanks
 *
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::ClearBanks()
{
	if (m_bSoundEngineInitialized)
	{
		FWwiseLowLevelSoundEngine* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (!SoundEngine)
		{
			return AK_NotInitialized;
		}
		return SoundEngine->ClearBanks();
	}

	return AK_NotInitialized;
}

AKRESULT FAkAudioDevice::LoadBank(
	const FString& in_BankName,
	AkBankID& out_bankID
)
{
	AKRESULT eResult = AK_Fail;
	if (EnsureInitialized()) // ensure audiolib is initialized
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->LoadBank(TCHAR_TO_AK(*in_BankName), out_bankID);
	}

	if (eResult != AK_Success)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::LoadBank: Failed to load bank %s. %s"), *in_BankName, AkUnrealHelper::GetResultString(eResult));
	}

	return eResult;
}

AKRESULT FAkAudioDevice::LoadBank(
	const FString&     in_BankName,
	AkBankCallbackFunc  in_pfnBankCallback,
	void *              in_pCookie,
	AkBankID &          out_bankID
)
{
	if (EnsureInitialized()) // ensure audiolib is initialized
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->LoadBank(TCHAR_TO_AK(*in_BankName), in_pfnBankCallback, in_pCookie, out_bankID);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::LoadBank(
	const FString&     in_BankName,
	FWaitEndBankAction* LoadBankLatentAction
)
{
	if (EnsureInitialized()) // ensure audiolib is initialized
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkBankID BankId;
		return SoundEngine->LoadBank(TCHAR_TO_AK(*in_BankName), nullptr, LoadBankLatentAction, BankId);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::LoadBankAsync(
	const FString&     in_BankName,
	const FOnAkBankCallback& BankLoadedCallback,
	AkBankID &          out_bankID
)
{
	if (EnsureInitialized()) // ensure audiolib is initialized
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->LoadBank(TCHAR_TO_AK(*in_BankName), out_bankID);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::LoadBankFromMemory(
	const void* MemoryPtr,
	uint32 MemorySize,
	AkBankType BankType,
	AkBankID& OutBankID
)
{
	if (EnsureInitialized() && MemoryPtr && MemorySize > 0)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->LoadBankMemoryView(MemoryPtr, MemorySize, OutBankID, BankType);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::UnloadBank(
	const FString&      in_BankName
    )
{
	AKRESULT eResult = AK_Fail;
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->UnloadBank(TCHAR_TO_AK(*in_BankName), nullptr );
	}
	if (eResult != AK_Success)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::UnloadBank: Failed to unload bank %s. %s"), *in_BankName, AkUnrealHelper::GetResultString(eResult));
	}
	return eResult;
}

AKRESULT FAkAudioDevice::UnloadBank(
	const FString&     in_BankName,
	AkBankCallbackFunc  in_pfnBankCallback,
	void *              in_pCookie
)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->UnloadBank(TCHAR_TO_AK(*in_BankName), NULL, in_pfnBankCallback, in_pCookie);
	}
	return AK_Fail;
}

AKRESULT FAkAudioDevice::UnloadBank(
	const FString&     in_BankName,
	FWaitEndBankAction* UnloadBankLatentAction
)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->UnloadBank(TCHAR_TO_AK(*in_BankName), nullptr);
	}
	return AK_Fail;
}

AKRESULT FAkAudioDevice::UnloadBankFromMemory(
	AkBankID in_bankID,
	const void* in_memoryPtr
)
{
	if (EnsureInitialized() && in_memoryPtr)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->UnloadBank(in_bankID, in_memoryPtr);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::UnloadBankFromMemoryAsync(
	AkBankID in_bankID,
	const void* in_memoryPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	uint32 BankType
)
{
	if (EnsureInitialized() && in_memoryPtr)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->UnloadBank(in_bankID, in_memoryPtr, in_pfnBankCallback, in_pCookie, BankType);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::UnloadBankAsync(
	const FString&     in_BankName,
	const FOnAkBankCallback& BankUnloadedCallback
)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->UnloadBank(TCHAR_TO_AK(*in_BankName), nullptr);
	}
	return AK_Fail;
}

AkUInt32 FAkAudioDevice::GetShortIDFromString(const FString& InString)
{
	if (InString.IsEmpty())
	{
		return AK_INVALID_UNIQUE_ID;
	}

	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_INVALID_UNIQUE_ID;

	return SoundEngine->GetIDFromString(TCHAR_TO_ANSI(*InString));
}

AkUInt32 FAkAudioDevice::GetShortID(UAkAudioType* AudioAsset, const FString& BackupName)
{
	AkUInt32 ShortId;
	if (AudioAsset)
	{
		ShortId = AudioAsset->GetShortID();
	}
	else
	{
		ShortId = GetShortIDFromString(BackupName);
	}

	if (ShortId == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetShortID : Returning invalid ShortId for Wwise Object named %s."), AudioAsset? *AudioAsset->GetName() : *BackupName);
	}

	return ShortId;
}

AKRESULT FAkAudioDevice::SetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->SetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

AKRESULT FAkAudioDevice::TryUnsetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings, AKRESULT* out_pUnsetResults)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->TryUnsetMedia(in_pSourceSettings, in_uNumSourceSettings, out_pUnsetResults);
}

AKRESULT FAkAudioDevice::UnsetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->UnsetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

FString FAkAudioDevice::GetCurrentAudioCulture() const
{
	auto* StreamMgr = FWwiseLowLevelStreamMgr::Get();
	if (UNLIKELY(!StreamMgr))
	{
		return {};
	}
	return FString(StreamMgr->GetCurrentLanguage());
}

FString FAkAudioDevice::GetDefaultLanguage() 
{
	auto* WwiseInitBankLoader = UWwiseInitBankLoader::Get();
	if (LIKELY(WwiseInitBankLoader))
	{
		auto* InitBankAsset = WwiseInitBankLoader->GetInitBankAsset();
		if (LIKELY(InitBankAsset))
		{
			TArray<FWwiseLanguageCookedData> Languages = InitBankAsset->GetLanguages();
			for (auto& Language : Languages)
			{
				if (Language.LanguageRequirement == EWwiseLanguageRequirement::IsDefault)
				{
					return Language.LanguageName;
				}
			}
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetDefaultLanguage: Could not get AkInitBank asset, returning empty language."));
		}
	}
	else
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetDefaultLanguage: Could not get WwiseInitBankLoader, returning empty language."));
	}

	UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetDefaultLanguage: Could not find default language in available languages list."));
	return {};
}

TArray<FString> FAkAudioDevice::GetAvailableAudioCultures() const
{
	auto* WwiseInitBankLoader = UWwiseInitBankLoader::Get();
	if (UNLIKELY(!WwiseInitBankLoader))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetAvailableAudioCultures: Could not get WwiseInitBankLoader, returning empty list."));
		return {};
	}

	auto* InitBankAsset = WwiseInitBankLoader->GetInitBankAsset();
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioDevice::GetAvailableAudioCultures: Could not get AkInitBank asset, returning empty list."));
		return {};
	}

	TSet<FString> LanguageNames;
	for(const auto& Language : InitBankAsset->InitBankCookedData.Language)
	{
		LanguageNames.Add(Language.LanguageName);
	}
	return LanguageNames.Array();
}

FWwiseLanguageCookedData FAkAudioDevice::GetLanguageCookedDataFromString(const FString& WwiseLanguage)
{
	auto* WwiseInitBankLoader = UWwiseInitBankLoader::Get();
	if (LIKELY(WwiseInitBankLoader))
	{
		auto* InitBankAsset = WwiseInitBankLoader->GetInitBankAsset();
		if (LIKELY(InitBankAsset))
		{
			for (auto& Language : InitBankAsset->InitBankCookedData.Language)
			{
				if (Language.LanguageName == WwiseLanguage)
				{
					return FWwiseLanguageCookedData(Language.LanguageId, Language.LanguageName, Language.LanguageRequirement);
				}
			}
		}
	}

	return FWwiseLanguageCookedData(GetShortIDFromString(WwiseLanguage), WwiseLanguage, EWwiseLanguageRequirement::IsOptional);
}

void FAkAudioDevice::SetCurrentAudioCulture(const FString& NewAudioCulture)
{
	FString NewWwiseLanguage;
	if (FindWwiseLanguage(NewAudioCulture, NewWwiseLanguage))
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		ResourceLoader->SetLanguage(GetLanguageCookedDataFromString(NewWwiseLanguage), EWwiseReloadLanguage::Immediate);

		auto* StreamMgr = FWwiseLowLevelStreamMgr::Get();
		if (UNLIKELY(!StreamMgr))
		{
			return;
		}
		StreamMgr->SetCurrentLanguage(TCHAR_TO_AK(*NewWwiseLanguage));
	}
}

void FAkAudioDevice::SetCurrentAudioCultureAsync(const FString& NewAudioCulture, FSetCurrentAudioCultureAction* LatentAction)
{
	FString NewWwiseLanguage;

	if (FindWwiseLanguage(NewAudioCulture, NewWwiseLanguage))
	{
		SetCurrentAudioCultureAsyncTask* newTask = new SetCurrentAudioCultureAsyncTask(GetLanguageCookedDataFromString(NewWwiseLanguage), LatentAction);
		if (newTask->Start())
		{
			AudioCultureAsyncTasks.Add(newTask);
		}
		else
		{
			LatentAction->ActionDone = true;
			delete newTask;
		}
	}
	else
	{
		LatentAction->ActionDone = true;
	}
}

void FAkAudioDevice::SetCurrentAudioCultureAsync(const FString& NewAudioCulture, const FOnSetCurrentAudioCultureCompleted& CompletedCallback)
{
	FString NewWwiseLanguage;

	if (FindWwiseLanguage(NewAudioCulture, NewWwiseLanguage))
	{
		SetCurrentAudioCultureAsyncTask* newTask = new SetCurrentAudioCultureAsyncTask(GetLanguageCookedDataFromString(NewWwiseLanguage), CompletedCallback);
		if (newTask->Start())
		{
			AudioCultureAsyncTasks.Add(newTask);
		}
		else
		{
			CompletedCallback.ExecuteIfBound(false);
			delete newTask;
		}
	}
	else
	{
		CompletedCallback.ExecuteIfBound(true);
	}
}

bool FAkAudioDevice::FindWwiseLanguage(const FString& NewAudioCulture, FString& FoundWwiseLanguage)
{
	auto oldAudioCulture = GetCurrentAudioCulture();

	FoundWwiseLanguage = NewAudioCulture;

	auto newCulturePtr = FInternationalization::Get().GetCulture(FoundWwiseLanguage);
	if (newCulturePtr && newCulturePtr->GetLCID() != InvariantLCID)
	{
		auto& newLanguage = newCulturePtr->GetTwoLetterISOLanguageName();
		auto& newRegion = newCulturePtr->GetRegion();
		auto& newScript = newCulturePtr->GetScript();

		int maxMatch = 0;

		auto findMostCompatibleCulture = [&maxMatch, &newLanguage, &newRegion, &newScript, &FoundWwiseLanguage](const FCulturePtr& currentCulture, const FString& currentValue) {
			auto& currentLanguageName = currentCulture->GetTwoLetterISOLanguageName();
			auto& currentRegionName = currentCulture->GetRegion();
			auto& currentScript = currentCulture->GetScript();

			int currentMatch = 0;

			if (currentLanguageName == newLanguage)
			{
				currentMatch = 1;

				// This is inspired by how UE processes culture from most to least specific. For example:
				// zh - Hans - CN is processed as "zh-Hans-CN", then "zh-CN", then "zh-Hans", then "zh".
				// This is how I selected the weight for each match.

				if (!currentScript.IsEmpty() && !newScript.IsEmpty()
					&& !currentRegionName.IsEmpty() && !newRegion.IsEmpty())
				{
					if (currentScript == newScript && currentRegionName == newRegion)
					{
						currentMatch += 4;
					}
					else
					{
						--currentMatch;
					}
				}

				if (!currentRegionName.IsEmpty() && !newRegion.IsEmpty())
				{
					if (currentRegionName == newRegion)
					{
						currentMatch += 3;
					}
					else
					{
						--currentMatch;
					}
				}

				if (!currentScript.IsEmpty() && !newScript.IsEmpty())
				{
					if (currentScript == newScript)
					{
						currentMatch += 2;
					}
					else
					{
						--currentMatch;
					}
				}
			}

			if (currentMatch > 0)
			{
				// When the current culture is missing script or region but the new culture
				// has it, give a weight boost of 1

				if (!newScript.IsEmpty() && currentScript.IsEmpty())
				{
					++currentMatch;
				}

				if (!newRegion.IsEmpty() && currentRegionName.IsEmpty())
				{
					++currentMatch;
				}
			}

			if (maxMatch < currentMatch)
			{
				FoundWwiseLanguage = currentValue;
				maxMatch = currentMatch;
			}
		};

#if WITH_EDITOR
		if (auto* akSettings = GetDefault<UAkSettings>())
		{
			for (auto& entry : akSettings->UnrealCultureToWwiseCulture)
			{
				auto currentCulture = FInternationalization::Get().GetCulture(entry.Key);
				if (currentCulture && currentCulture->GetLCID() != InvariantLCID)
				{
					findMostCompatibleCulture(currentCulture, entry.Value);
				}
			}
		}
#else
		for (auto& entry : CachedUnrealToWwiseCulture)
		{
			findMostCompatibleCulture(entry.Key, entry.Value);
		}
#endif
	}

	return oldAudioCulture != FoundWwiseLanguage;
}

void FAkAudioDevice::UpdateSetCurrentAudioCultureAsyncTasks()
{
	if(AudioCultureAsyncTasks.Num() == 0)
	{
		return;
	}

	for (auto task : AudioCultureAsyncTasks)
	{
		task->Update();
	}

	for (int32 i = AudioCultureAsyncTasks.Num() - 1; i >= 0; --i)
	{
		if (AudioCultureAsyncTasks[i]->IsDone)
		{
			delete AudioCultureAsyncTasks[i];
			AudioCultureAsyncTasks[i] = nullptr;
		}
	}

	AudioCultureAsyncTasks.RemoveAll([](SetCurrentAudioCultureAsyncTask* Task) { return Task == nullptr; });
}

template<typename FCreateCallbackPackage>
AkPlayingID FAkAudioDevice::PostEventWithCallbackPackageOnGameObjectId(
	const AkUInt32 EventShortID,
	const AkGameObjectID GameObjectID,
	const TArray<AkExternalSourceInfo>& ExternalSources,
	FCreateCallbackPackage CreateCallbackPackage
)
{
	AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
	if (m_bSoundEngineInitialized && CallbackManager)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		auto pPackage = CreateCallbackPackage(GameObjectID);
		if (pPackage)
		{
			PlayingID = SoundEngine->PostEvent(
				  EventShortID
				, GameObjectID
				, pPackage->uUserFlags | AK_EndOfEvent
				, &FAkComponentCallbackManager::AkComponentCallback
				, pPackage
				, ExternalSources.Num()
				, const_cast<AkExternalSourceInfo*>(ExternalSources.GetData())
			);
			if (PlayingID == AK_INVALID_PLAYING_ID)
			{
				CallbackManager->RemoveCallbackPackage(pPackage, GameObjectID);
			}
			else
			{
				FScopeLock Lock(&EventToPlayingIDMapCriticalSection);
				auto& PlayingIDArray = EventToPlayingIDMap.FindOrAdd(EventShortID);
				PlayingIDArray.Add(PlayingID);

				if (ExternalSources.Num() >0)
				{
					if (auto* ExternalSourceMananger = IWwiseExternalSourceManager::Get())
					{
						ExternalSourceMananger->OnPostEvent(PlayingID, ExternalSources);
					}
				}
			}
		}
	}

	return PlayingID;
}

template<typename FCreateCallbackPackage>
AkPlayingID FAkAudioDevice::PostEventWithCallbackPackageOnAkGameObject(
	const AkUInt32 EventShortID,
	UAkGameObject* GameObject,
	const TArray<AkExternalSourceInfo>& ExternalSources,
	FCreateCallbackPackage CreateCallbackPackage
)
{
	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	if (m_bSoundEngineInitialized && GameObject && CallbackManager)
	{
		if (EventShortID != AK_INVALID_UNIQUE_ID && GameObject->AllowAudioPlayback())
		{
			GameObject->UpdateOcclusionObstruction();

			auto gameObjID = GameObject->GetAkGameObjectID();

			return PostEventWithCallbackPackageOnGameObjectId(EventShortID, gameObjID, ExternalSources, CreateCallbackPackage);
		}
	}

	return playingID;
}

/**
 * Post an event to ak soundengine
 *
 * @param AkEvent			Event to post
 * @param in_pComponent		AkComponent on which to play the event
 * @param Flags			Bitmask: see \ref AkCallbackType
 * @param Callback	Callback function
 * @param Cookie		Callback cookie that will be sent to the callback function along with additional information.
 * @param bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
 * @return ID assigned by ak soundengine
 */
AkPlayingID FAkAudioDevice::PostAkAudioEventOnActor(
	UAkAudioEvent * AkEvent, 
	AActor * Actor,
	AkUInt32 Flags /*= 0*/,
	AkCallbackFunc Callback /*= NULL*/,
	void * Cookie /*= NULL*/,
	bool bStopWhenOwnerDestroyed, /*= false*/
	TArray<AkExternalSourceInfo> ExternalSources /* = TArray<AkExternalSourceInfo>()*/
    )
{
	if (!AkEvent)
		return AK_INVALID_PLAYING_ID;

	if (!AkEvent->IsDataFullyLoaded())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Not all localization data for '%s' are loaded. Consider using PostEventAsync()."), *AkEvent->GetName());
	}

	if (ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(ExternalSources, AkEvent->GetExternalSources());
	}
	AkPlayingID playingID = PostEventOnActor(AkEvent->GetShortID(), Actor, Flags, Callback, Cookie, bStopWhenOwnerDestroyed, ExternalSources);
	return playingID;
}

AkPlayingID FAkAudioDevice::PostAkAudioEventOnComponent(
	UAkAudioEvent* AkEvent, 
	UAkComponent* Component, 
	AkUInt32 Flags,
	AkCallbackFunc Callback,
	void* Cookie,
	bool bStopWhenOwnerDestroyed,
	TArray<AkExternalSourceInfo> ExternalSources)
{
	if (!AkEvent)
	{
		return AK_INVALID_PLAYING_ID;
	}

	if (!AkEvent->IsDataFullyLoaded())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Not all data for '%s' are loaded. Consider using PostEventAsync()."), *AkEvent->GetName());
	}

	if (ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(ExternalSources, AkEvent->GetExternalSources());
	}

	if (!Component->IsBeingDestroyed() && IsValid(Component))
	{
		Component->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
		AkPlayingID playingID = PostEventOnAkComponent(AkEvent->GetShortID(), Component, Flags, Callback, Cookie, ExternalSources);
		return playingID;
	}

	return AK_INVALID_PLAYING_ID;
}

/**
 * Post an event to ak soundengine by name
 *
 * @param EventShortID		Name of the event to post
 * @param in_pComponent		AkComponent on which to play the event
 * @param Flags			Bitmask: see \ref AkCallbackType
 * @param Callback	Callback function
 * @param Cookie		Callback cookie that will be sent to the callback function along with additional information.
 * @param bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
 * @return ID assigned by ak soundengine
 */
AkPlayingID FAkAudioDevice::PostEventOnActor(
	const AkUInt32 EventShortID,
	AActor * Actor,
	AkUInt32 Flags /*= 0*/,
	AkCallbackFunc Callback /*= NULL*/,
	void * Cookie /*= NULL*/,
	bool bStopWhenOwnerDestroyed /*= false*/,
	TArray<AkExternalSourceInfo> ExternalSources /* = TArray<AkExternalSourceInfo>()*/
)
{
	if (m_bSoundEngineInitialized)
	{
		if (!Actor)
		{
			return PostEventWithCallbackPackageOnGameObjectId(EventShortID, DUMMY_GAMEOBJ, ExternalSources, [Callback, Cookie, Flags, this, HasExtSrc=ExternalSources.Num()>0](AkGameObjectID gameObjID) {
				return CallbackManager->CreateCallbackPackage(Callback, Cookie, Flags, gameObjID, HasExtSrc);
			});
		}
		else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
		{
			UAkComponent* pComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
			if (pComponent)
			{
				pComponent->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
				return PostEventOnAkComponent(EventShortID, pComponent, Flags, Callback, Cookie, ExternalSources);
			}
		}
	}

	return AK_INVALID_PLAYING_ID;
}

AkPlayingID FAkAudioDevice::PostEventOnActor(
	const AkUInt32 EventShortID,
	AActor * Actor,
	const FOnAkPostEventCallback& PostEventCallback,
	AkUInt32 Flags /*= 0*/,
	bool bStopWhenOwnerDestroyed,
	/*= false*/
	const TArray<AkExternalSourceInfo>& ExternalSources /* = TArray<AkExternalSourceInfo>()*/
)
{
	if (m_bSoundEngineInitialized)
	{
		if (!Actor)
		{
			UE_LOG(LogAkAudio, Error, TEXT("PostEvent accepting a FOnAkPostEventCallback delegate requires a valid actor"));
		}
		else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
		{
			UAkComponent* pComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
			if (pComponent)
			{
				pComponent->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
				return PostEventOnAkGameObject(EventShortID, pComponent, PostEventCallback, Flags, ExternalSources);
			}
		}
	}

	return AK_INVALID_PLAYING_ID;
}

AkPlayingID FAkAudioDevice::PostEventOnActorWithLatentAction(
	const AkUInt32 EventShortID,
	AActor * Actor,
	bool bStopWhenOwnerDestroyed,
	FWaitEndOfEventAction* LatentAction,
	const TArray<AkExternalSourceInfo>& ExternalSources /* = TArray<AkExternalSourceInfo>()*/
)
{
	if (m_bSoundEngineInitialized)
	{
		if (!Actor)
		{
			UE_LOG(LogAkAudio, Error, TEXT("PostEvent accepting a FWaitEndOfEventAction requires a valid actor"));
		}
		else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
		{
			UAkComponent* pComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
			if (pComponent)
			{
				pComponent->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
				return PostEventOnComponentWithLatentAction(EventShortID, pComponent, LatentAction, ExternalSources);
			}
		}
	}

	return AK_INVALID_PLAYING_ID;
}

/**
 * Post an event to ak soundengine by name
 *
 * @param EventShortID		Name of the event to post
 * @param in_pGameObject	UAkGameObject on which to play the event
 * @param in_uFlags			Bitmask: see \ref AkCallbackType
 * @param in_pfnCallback	Callback function
 * @param in_pCookie		Callback cookie that will be sent to the callback function along with additional information.
 * @return ID assigned by ak soundengine
 */
AkPlayingID FAkAudioDevice::PostEventOnAkComponent(
	const AkUInt32 EventShortID,
	UAkComponent* Component,
	AkUInt32 Flags /*= 0*/,
	AkCallbackFunc Callback /*= NULL*/,
	void * Cookie,
	/*= NULL*/
	const TArray<AkExternalSourceInfo>& ExternalSources /*= TArray<AkExternalSourceInfo>()*/
)
{
	return PostEventWithCallbackPackageOnAkGameObject(EventShortID, Component, ExternalSources, [Callback, Cookie, Flags, this, HasExtSrc = ExternalSources.Num() > 0](AkGameObjectID gameObjID) {
		return CallbackManager->CreateCallbackPackage(Callback, Cookie, Flags, gameObjID, HasExtSrc);
	});
}

AkPlayingID FAkAudioDevice::PostEventOnGameObjectID(
	const AkUInt32 EventShortID,
	AkGameObjectID GameObject,
	AkUInt32 Flags /*= 0*/,
	AkCallbackFunc Callback /*= NULL*/,
	void* Cookie,
	/*= NULL*/
	const TArray<AkExternalSourceInfo>&ExternalSources /*= TArray<AkExternalSourceInfo>()*/
)
{
	return PostEventWithCallbackPackageOnGameObjectId(EventShortID, GameObject, ExternalSources, [Callback, Cookie, Flags, this, HasExtSrc = ExternalSources.Num() > 0](AkGameObjectID gameObjID) {
		return CallbackManager->CreateCallbackPackage(Callback, Cookie, Flags, gameObjID, HasExtSrc);
	});
}

AkPlayingID FAkAudioDevice::PostEventOnAkGameObject(
	const AkUInt32 EventShortID,
	UAkGameObject* AkGameObject,
	const FOnAkPostEventCallback& PostEventCallback,
	AkUInt32 Flags,
	/*= 0*/
	const TArray<AkExternalSourceInfo>& ExternalSources /*= TArray<AkExternalSourceInfo>()*/
)
{
	return PostEventWithCallbackPackageOnAkGameObject(EventShortID, AkGameObject, ExternalSources, [PostEventCallback, Flags, this, HasExtSrc = ExternalSources.Num() > 0](AkGameObjectID gameObjID) {
		return CallbackManager->CreateCallbackPackage(PostEventCallback, Flags, gameObjID, HasExtSrc);
	});
}

AkPlayingID FAkAudioDevice::PostEventOnComponentWithLatentAction(
	const AkUInt32 EventShortID,
	UAkComponent* AkComponent,
	FWaitEndOfEventAction* LatentAction,
	const TArray<AkExternalSourceInfo>& ExternalSources /*= TArray<AkExternalSourceInfo>()*/
)
{
	return PostEventWithCallbackPackageOnAkGameObject(EventShortID, AkComponent, ExternalSources, [LatentAction, this, HasExtSrc = ExternalSources.Num() > 0](AkGameObjectID gameObjID) {
		return CallbackManager->CreateCallbackPackage(LatentAction, gameObjID, HasExtSrc);
	});
}

void FAkAudioDevice::SAComponentAddedRemoved(UWorld* World)
{
	if (World != nullptr)
	{
		if (WorldVolumesUpdatedMap.Contains(World))
			WorldVolumesUpdatedMap[World] = true;
		else
			WorldVolumesUpdatedMap.Add(World, true);
	}
}


TFuture<AkPlayingID> FAkAudioDevice::PostAkAudioEventOnActorAsync(
	UAkAudioEvent* AudioEvent, 
	AActor* Actor, 
	const FOnAkPostEventCallback& PostEventCallback, 
	AkUInt32 CallbackFlags, 
	bool bStopWhenOwnerDestroyed,
	TArray<AkExternalSourceInfo> in_ExternalSources
)
{
	TPromise<AkPlayingID> PlayingIDPromise;
	auto PlayingIDFuture = PlayingIDPromise.GetFuture();

	if (AudioEvent && in_ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(in_ExternalSources, AudioEvent->GetExternalSources());
	}

	auto PollMediaReadyTask = FFunctionGraphTask::CreateAndDispatchWhenReady([AudioEvent]()
		{
			while (AudioEvent && !AudioEvent->IsDataFullyLoaded())
			{
				FPlatformProcess::Sleep(1.f / 60.f);
			}
		}, GET_STATID(STAT_PostEventAsync), nullptr, ENamedThreads::AnyThread);

	FFunctionGraphTask::CreateAndDispatchWhenReady([this, AudioEvent, Actor, PostEventCallback, CallbackFlags, bStopWhenOwnerDestroyed, PlayingIDPromiseCopy(MoveTemp(PlayingIDPromise))]() mutable 
		{
			AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;

			if (m_bSoundEngineInitialized && AudioEvent && IsValid(AudioEvent))
			{
				if (!Actor)
				{
					UE_LOG(LogAkAudio, Error, TEXT("PostEvent accepting a FOnAkPostEventCallback delegate requires a valid actor"));
				}
				else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
				{
					UAkComponent* AkComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
					if (AkComponent)
					{
						AkComponent->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
						PlayingID = PostEventOnAkGameObject(AudioEvent->GetShortID(), AkComponent, PostEventCallback, CallbackFlags);
					}
				}
			}

			PlayingIDPromiseCopy.SetValue(PlayingID);
		}, GET_STATID(STAT_PostEventAsync), PollMediaReadyTask, ENamedThreads::GameThread);

	return PlayingIDFuture;
}


TFuture<AkPlayingID> FAkAudioDevice::PostAkAudioEventOnAkGameObjectAsync(
	UAkAudioEvent* AudioEvent,
	UAkGameObject* GameObject,
	const FOnAkPostEventCallback& PostEventCallback,
	AkUInt32 CallbackFlags, 
	TArray<AkExternalSourceInfo> in_ExternalSources)
{
	if (AudioEvent && in_ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(in_ExternalSources, AudioEvent->GetExternalSources());
	}

	auto PlayingIDFuture = Async(EAsyncExecution::TaskGraph, [AudioEvent] {
		while (AudioEvent && !AudioEvent->IsDataFullyLoaded())
		{
			FPlatformProcess::Sleep(1.f / 60.f);
		}
	}).Then([this, AudioEvent, GameObject, PostEventCallback, CallbackFlags, HasExtSrc = in_ExternalSources.Num() > 0](auto PreviousFuture) {
		if (!AudioEvent || !IsValid(AudioEvent))
		{
			return AK_INVALID_PLAYING_ID;
		}
		AkPlayingID PlayingID = PostEventWithCallbackPackageOnAkGameObject(AudioEvent->GetShortID(), GameObject, TArray<AkExternalSourceInfo>(),
			[PostEventCallback, CallbackFlags, this, HasExtSrc](AkGameObjectID GameObjectID)
		{
			return CallbackManager->CreateCallbackPackage(PostEventCallback, CallbackFlags, GameObjectID, HasExtSrc);
		});

		return PlayingID;
	});

	return PlayingIDFuture;
}

TFuture<AkPlayingID> FAkAudioDevice::PostAkAudioEventAtLocationAsync(
	class UAkAudioEvent* Event,
	FVector Location,
	FRotator Orientation,
	class UWorld* World
)
{
	TPromise<AkPlayingID> playingIDPromise;
	auto playingIDFuture = playingIDPromise.GetFuture();

	auto pollMediaReadyTask = FFunctionGraphTask::CreateAndDispatchWhenReady([Event]()
		{
			while (Event && !Event->IsDataFullyLoaded())
			{
				FPlatformProcess::Sleep(1.f / 60.f);
			}
		}, GET_STATID(STAT_PostEventAsync), nullptr, ENamedThreads::AnyThread);

	FFunctionGraphTask::CreateAndDispatchWhenReady([this, Event, Location, Orientation, World, playingIDPromiseCopy(MoveTemp(playingIDPromise))]() mutable {
		AkPlayingID playingID = AK_INVALID_PLAYING_ID;

		if (Event && IsValid(Event))
		{
			playingID = PostEventAtLocation(Event->GetName(), Event->GetShortID(), Location, Orientation, World);
		}

		playingIDPromiseCopy.SetValue(playingID);
	}, GET_STATID(STAT_PostEventAsync), pollMediaReadyTask, ENamedThreads::GameThread);

	return playingIDFuture;
}

TFuture<AkPlayingID> FAkAudioDevice::PostAkAudioEventWithLatentActionOnActorAsync(
	UAkAudioEvent* AudioEvent,
	AActor* Actor,
	bool bStopWhenOwnerDestroyed,
	FWaitEndOfEventAction* LatentAction, 
	TArray<AkExternalSourceInfo> in_ExternalSources)
{
	if (AudioEvent && in_ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(in_ExternalSources, AudioEvent->GetExternalSources());
	}

	TPromise<AkPlayingID> PlayingIDPromise;
	auto PlayingIDFuture = PlayingIDPromise.GetFuture();

	auto PollMediaReadyTask = FFunctionGraphTask::CreateAndDispatchWhenReady([AudioEvent]()
		{
			while (AudioEvent && !AudioEvent->IsDataFullyLoaded())
			{
				FPlatformProcess::Sleep(1.f / 60.f);
			}
		}, GET_STATID(STAT_PostEventAsync), nullptr, ENamedThreads::AnyThread);

	FFunctionGraphTask::CreateAndDispatchWhenReady([this, AudioEvent, Actor, bStopWhenOwnerDestroyed, LatentAction,in_ExternalSources, PlayingIDPromiseCopy(MoveTemp(PlayingIDPromise))]() mutable {
		AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;

		if (m_bSoundEngineInitialized && AudioEvent && IsValid(AudioEvent))
		{
			if (!Actor)
			{
				UE_LOG(LogAkAudio, Error, TEXT("PostEvent accepting a FWaitEndOfEventAction requires a valid actor"));
			}
			else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
			{
				UAkComponent* AkComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
				if (AkComponent)
				{
					AkComponent->StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
					PlayingID = PostEventWithCallbackPackageOnAkGameObject(AudioEvent->GetShortID(), AkComponent, TArray<AkExternalSourceInfo>(),
						[LatentAction, this, HasExtSrc = in_ExternalSources.Num() > 0](AkGameObjectID GameObjectID)
					{
						return CallbackManager->CreateCallbackPackage(LatentAction, GameObjectID, HasExtSrc);
					});
				}
			}
		}

		PlayingIDPromiseCopy.SetValue(PlayingID);
	}, GET_STATID(STAT_PostEventAsync), PollMediaReadyTask, ENamedThreads::GameThread);

	return PlayingIDFuture;
}

TFuture<AkPlayingID> FAkAudioDevice::PostAkAudioEventWithLatentActionOnAkComponentAsync(
	UAkAudioEvent* AudioEvent,
	UAkComponent* AkComponent,
	FWaitEndOfEventAction* LatentAction, 
	TArray<AkExternalSourceInfo> ExternalSources)
{
	if (AudioEvent && ExternalSources.Num() == 0)
	{
		IWwiseExternalSourceManager::Get()->GetExternalSourceInfos(ExternalSources, AudioEvent->GetExternalSources());
	}

	auto PlayingIDFuture = Async(EAsyncExecution::TaskGraph, [AudioEvent] 
		{
			while (AudioEvent && !AudioEvent->IsDataFullyLoaded())
			{
				FPlatformProcess::Sleep(1.f / 60.f);
			}
		}).Then([this, AudioEvent, AkComponent, LatentAction, ExternalSources](auto PreviousFuture)
		{
			if (m_bSoundEngineInitialized && AudioEvent && IsValid(AudioEvent))
			{
				AkPlayingID PlayingID = PostEventWithCallbackPackageOnAkGameObject(AudioEvent->GetShortID(), AkComponent, ExternalSources,
					[LatentAction, this, HasExtSrc = ExternalSources.Num() > 0](AkGameObjectID GameObjectID)
					{
						return CallbackManager->CreateCallbackPackage(LatentAction, GameObjectID, HasExtSrc);
					});

				return PlayingID;
			}
			return AK_INVALID_PLAYING_ID;
		});

	return PlayingIDFuture;
}

/** Find UAkLateReverbComponents at a given location. */
TArray<class UAkLateReverbComponent*> FAkAudioDevice::FindLateReverbComponentsAtLocation(const FVector& Loc, const UWorld* World)
{
	return LateReverbIndex.Query<UAkLateReverbComponent>(Loc, World);
}

/** Add a UAkLateReverbComponent to the linked list. */
void FAkAudioDevice::IndexLateReverb(class UAkLateReverbComponent* ComponentToAdd)
{
	check(!ComponentToAdd->IsIndexed);
	LateReverbIndex.Add(ComponentToAdd);
	SAComponentAddedRemoved(ComponentToAdd->GetWorld());
	ComponentToAdd->IsIndexed = true;
}

/** Remove a UAkLateReverbComponent from the linked list. */
void FAkAudioDevice::UnindexLateReverb(class UAkLateReverbComponent* ComponentToRemove)
{
	check(ComponentToRemove->IsIndexed);
	if (LateReverbIndex.Remove(ComponentToRemove))
	{
		SAComponentAddedRemoved(ComponentToRemove->GetWorld());
	}
	ComponentToRemove->IsIndexed = false;
}

void FAkAudioDevice::ReindexLateReverb(class UAkLateReverbComponent* ComponentToUpdate)
{
	check(ComponentToUpdate->IsIndexed);
	LateReverbIndex.Update(ComponentToUpdate);
	SAComponentAddedRemoved(ComponentToUpdate->GetWorld());
}

bool FAkAudioDevice::WorldHasActiveRooms(UWorld* World)
{
	return !RoomIndex.IsEmpty(World);
}

/** Find UAkRoomComponents at a given location. */
TArray<class UAkRoomComponent*> FAkAudioDevice::FindRoomComponentsAtLocation(const FVector& Loc, const UWorld* World)
{
	return RoomIndex.Query<UAkRoomComponent>(Loc, World);
}

/** Add a UAkRoomComponent to the linked list. */
void FAkAudioDevice::IndexRoom(class UAkRoomComponent* ComponentToAdd)
{
	RoomIndex.Add(ComponentToAdd);
	SAComponentAddedRemoved(ComponentToAdd->GetWorld());
}

/** Remove a UAkRoomComponent from the linked list. */
void FAkAudioDevice::UnindexRoom(class UAkRoomComponent* ComponentToRemove)
{
	if (RoomIndex.Remove(ComponentToRemove))
	{
		SAComponentAddedRemoved(ComponentToRemove->GetWorld());
	}
}

void FAkAudioDevice::ReindexRoom(class UAkRoomComponent* ComponentToAdd)
{
	RoomIndex.Update(ComponentToAdd);
	SAComponentAddedRemoved(ComponentToAdd->GetWorld());
}

/** Return true if any UAkRoomComponents have been added to the prioritized list of rooms **/
bool FAkAudioDevice::UsingSpatialAudioRooms(const UWorld* World)
{
	return !RoomIndex.IsEmpty(World);
}

AKRESULT FAkAudioDevice::ExecuteActionOnEvent(
	const AkUInt32 EventShortID,
	AkActionOnEventType ActionType,
	AActor* Actor,
	AkTimeMs TransitionDuration,
	EAkCurveInterpolation FadeCurve,
	AkPlayingID PlayingID
)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	if (!Actor)
	{
		return SoundEngine->ExecuteActionOnEvent(EventShortID,
			static_cast<AK::SoundEngine::AkActionOnEventType>(ActionType),
			DUMMY_GAMEOBJ,
			TransitionDuration,
			static_cast<AkCurveInterpolation>(FadeCurve),
			PlayingID
		);
	}
	else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
	{
		UAkComponent* pComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
		if (pComponent)
		{
			return SoundEngine->ExecuteActionOnEvent(EventShortID,
				static_cast<AK::SoundEngine::AkActionOnEventType>(ActionType),
				pComponent->GetAkGameObjectID(),
				TransitionDuration,
				static_cast<AkCurveInterpolation>(FadeCurve),
				PlayingID
			);
		}
	}

	return AKRESULT::AK_Fail;
}

void FAkAudioDevice::ExecuteActionOnPlayingID(
	AkActionOnEventType in_ActionType,
	AkPlayingID in_PlayingID,
	AkTimeMs in_uTransitionDuration,
	EAkCurveInterpolation in_eFadeCuve
)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	SoundEngine->ExecuteActionOnPlayingID(
		static_cast<AK::SoundEngine::AkActionOnEventType>(in_ActionType),
		in_PlayingID,
		in_uTransitionDuration,
		static_cast<AkCurveInterpolation>(in_eFadeCuve)
	);
}

/** Seek on an event in the ak soundengine.
* @param EventShortID            Name of the event on which to seek.
* @param Actor               The associated Actor. If this is nullptr, defaul object will be used.
* @param in_fPercent             Desired percent where playback should restart.
* @param in_bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
*
* @return Success or failure.
*/
AKRESULT FAkAudioDevice::SeekOnEvent(
	const AkUInt32 EventShortID,
	AActor* Actor,
	AkReal32 Percent,
	bool bSeekToNearestMarker /*= false*/,
	AkPlayingID PlayingID      /*= AK_INVALID_PLAYING_ID*/
)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	if (!Actor)
    {
        // SeekOnEvent must be bound to a game object. Passing DUMMY_GAMEOBJ as default game object.
        return SoundEngine->SeekOnEvent(EventShortID, DUMMY_GAMEOBJ, Percent, bSeekToNearestMarker, PlayingID);
    }
    else if (!Actor->IsActorBeingDestroyed() && IsValid(Actor))
    {
        UAkComponent* pComponent = GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
        if (pComponent)
        {
            return SeekOnEvent(EventShortID, pComponent, Percent, bSeekToNearestMarker, PlayingID);
        }
    }

    return AKRESULT::AK_Fail;
}

/** Seek on an event in the ak soundengine.
* @param EventShortID            Name of the event on which to seek.
* @param Component           The associated AkComponent.
* @param in_fPercent             Desired percent where playback should restart.
* @param in_bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
*
* @return Success or failure.
*/
AKRESULT FAkAudioDevice::SeekOnEvent(
	const AkUInt32 EventShortID,
	UAkComponent* Component,
	AkReal32 Percent,
	bool bSeekToNearestMarker /*= false*/,
	AkPlayingID PlayingID      /*= AK_INVALID_PLAYING_ID*/
)
{
    if (m_bSoundEngineInitialized && Component)
    {
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		if (Component->AllowAudioPlayback())
        {
            return SoundEngine->SeekOnEvent(EventShortID, Component->GetAkGameObjectID(), Percent, bSeekToNearestMarker, PlayingID);
        }
    }
    return AKRESULT::AK_Fail;
}

void FAkAudioDevice::UpdateAllSpatialAudioRooms(UWorld* InWorld)
{
	for (TObjectIterator<UAkRoomComponent> Itr; Itr; ++Itr)
	{
		if (IsValid(*Itr) && Itr->GetWorld() == InWorld)
		{
			Itr->UpdateSpatialAudioRoom();
		}
	}
}

void FAkAudioDevice::UpdateAllSpatialAudioPortals(UWorld* InWorld)
{
#ifdef AK_ENABLE_PORTALS
	auto Portals = WorldPortalsMap.Find(InWorld);
	if (Portals != nullptr)
	{
		for (auto Portal : *Portals)
		{
			SetSpatialAudioPortal(Portal);
		}
	}
#endif
}

void FAkAudioDevice::SetSpatialAudioPortal(UAkPortalComponent* in_Portal)
{
	if(!IsValid(in_Portal) || IsRunningCommandlet())
		return;

	auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
	if (UNLIKELY(!SpatialAudio)) return;

#ifdef AK_ENABLE_PORTALS
	UWorld* World = in_Portal->GetWorld();

	auto Portals = WorldPortalsMap.Find(World);
	if (Portals == nullptr)
		Portals = &WorldPortalsMap.Add(World, TArray<UAkPortalComponent*>());
	if (Portals != nullptr)
	{
		if (!Portals->Contains(in_Portal))
		{
			Portals->Add(in_Portal);
		}
	}

	/* Only update the sound engine for game worlds. */
	if (ShouldNotifySoundEngine(World->WorldType))
	{
		AkPortalID portalID = in_Portal->GetPortalID();

		if (!in_Portal->PortalPlacementValid())
		{
			SpatialAudio->RemovePortal(portalID);
			UE_LOG(LogAkAudio, Warning, TEXT("UAkPortalComponent %s must have a front room which is distinct from its back room."), *(in_Portal->GetOwner() != nullptr ? in_Portal->GetOwner()->GetName() : in_Portal->GetName()));
		}
		else
		{
			FString nameStr = in_Portal->GetName();
			AActor* portalOwner = in_Portal->GetOwner();
			UPrimitiveComponent* primitiveParent = in_Portal->GetPrimitiveParent();
			if (portalOwner != nullptr)
			{
				nameStr = portalOwner->GetName();
				if (primitiveParent != nullptr)
				{
					// ensures unique and meaningful names when we have multiple portals in the same actor.
					TInlineComponentArray<UAkPortalComponent*> PortalComponents;
					portalOwner->GetComponents(PortalComponents);
					if (PortalComponents.Num() > 1)
						nameStr.Append(FString("_").Append(primitiveParent->GetName()));
				}
			}

			AkPortalParams Params;
			UPrimitiveComponent* Parent = in_Portal->GetPrimitiveParent();
			if (IsValid(Parent))
			{
				AkComponentHelpers::GetPrimitiveTransformAndExtent(*Parent, Params.Transform, Params.Extent);
			}

			Params.bEnabled = in_Portal->GetCurrentState() == AkAcousticPortalState::Open;
			Params.FrontRoom = in_Portal->GetFrontRoom();
			Params.BackRoom = in_Portal->GetBackRoom();

			SpatialAudio->SetPortal(portalID, Params, TCHAR_TO_ANSI(*nameStr));
		}
	}
#endif
}

void FAkAudioDevice::RemoveSpatialAudioPortal(UAkPortalComponent* in_Portal)
{
	if (IsRunningCommandlet())
		return;
	auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
	if (UNLIKELY(!SpatialAudio)) return;

#ifdef AK_ENABLE_PORTALS
	auto Portals = WorldPortalsMap.Find(in_Portal->GetWorld());
	if (Portals != nullptr && Portals->Contains(in_Portal))
	{
		Portals->Remove(in_Portal);
	}

	if (ShouldNotifySoundEngine(in_Portal->GetWorld()->WorldType))
	{
		AkPortalID portalID = in_Portal->GetPortalID();
		SpatialAudio->RemovePortal(portalID);
	}

	UpdateAllSpatialAudioRooms(in_Portal->GetWorld());
#endif
}

/** Get a sorted list of AkAuxSendValue at a location
 *
 * @param					Loc	Location at which to find Reverb Volumes
 * @param AkReverbVolumes	Array of AkAuxSendValue at this location
 */
void FAkAudioDevice::GetAuxSendValuesAtLocation(FVector Loc, TArray<AkAuxSendValue>& AkAuxSendValues, const UWorld* in_World)
{
	// Check if there are AkReverbVolumes at this location
	TArray<UAkLateReverbComponent*> FoundComponents = LateReverbIndex.Query<UAkLateReverbComponent>(Loc, in_World);

	// Sort the found Volumes
	if (FoundComponents.Num() > 1)
	{
		FoundComponents.Sort([](const UAkLateReverbComponent& A, const UAkLateReverbComponent& B)
		{
			return A.Priority > B.Priority;
		});
	}

	// Apply the found Aux Sends
	AkAuxSendValue	TmpSendValue;
	// Build a list to set as AuxBusses
	for( uint8 Idx = 0; Idx < FoundComponents.Num() && Idx < MaxAuxBus; Idx++ )
	{
		TmpSendValue.listenerID = AK_INVALID_GAME_OBJECT;
		TmpSendValue.auxBusID = FoundComponents[Idx]->GetAuxBusId();
		TmpSendValue.fControlValue = FoundComponents[Idx]->SendLevel;
		AkAuxSendValues.Add(TmpSendValue);
	}
}

/**
 * Post an event and location to ak soundengine
 *
 * @param Event			Name of the event to post
 * @param in_Location		Location at which to play the event
 * @return ID assigned by ak soundengine
 */
AkPlayingID FAkAudioDevice::PostAkAudioEventAtLocation(
	UAkAudioEvent * Event,
	FVector Location,
	FRotator Orientation,
	UWorld* World)
{
	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	if ( Event )
	{
		if (!Event->IsDataFullyLoaded())
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Not all data for '%s' are loaded. Consider using PostEventAsync()."), *Event->GetName());
		}
		playingID = PostEventAtLocation(Event->GetName(), Event->GetShortID(), Location, Orientation, World);
		return playingID;
	}

	return playingID;
}

void FAkAudioDevice::PostEventAtLocationEndOfEventCallback(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo)
{
	if (auto* Device = FAkAudioDevice::Get())
	{
		Device->RemovePlayingID(((AkEventCallbackInfo*)in_pCallbackInfo)->eventID, ((AkEventCallbackInfo*)in_pCallbackInfo)->playingID);
	}
}

/**
 * Post an event by name at location to ak soundengine
 *
 * @param in_pEvent			Name of the event to post
 * @param Location		Location at which to play the event
 * @return ID assigned by ak soundengine
 */
AkPlayingID FAkAudioDevice::PostEventAtLocation(
	const FString& EventName,
	const AkUInt32 EventShortID,
	FVector Location,
	FRotator Orientation, UWorld* World)
{
	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_INVALID_PLAYING_ID;

		const AkGameObjectID objId = (AkGameObjectID)&EventName;
		FAkAudioDevice_Helpers::RegisterGameObject(objId, EventName);

		TArray<AkAuxSendValue> AkReverbVolumes;
		GetAuxSendValuesAtLocation(Location, AkReverbVolumes, World);
		SoundEngine->SetGameObjectAuxSendValues(objId, AkReverbVolumes.GetData(), AkReverbVolumes.Num());

		AkRoomID RoomID;
		TArray<UAkRoomComponent*> AkRooms = RoomIndex.Query<UAkRoomComponent>(Location, World);
		if (AkRooms.Num() > 0)
			RoomID = AkRooms[0]->GetRoomID();

		SetInSpatialAudioRoom(objId, RoomID);

		AkSoundPosition soundpos;
		FQuat tempQuat(Orientation);
		FVectorsToAKWorldTransform(Location, tempQuat.GetForwardVector(), tempQuat.GetUpVector(), soundpos);

		SoundEngine->SetPosition(objId, soundpos);

		playingID = SoundEngine->PostEvent(EventShortID, objId, AK_EndOfEvent, &FAkAudioDevice::PostEventAtLocationEndOfEventCallback);
		if (playingID != AK_INVALID_PLAYING_ID)
		{
			FScopeLock Lock(&EventToPlayingIDMapCriticalSection);
			auto& PlayingIDs = EventToPlayingIDMap.FindOrAdd(EventShortID);
			PlayingIDs.Add(playingID);
		}
		SoundEngine->UnregisterGameObj( objId );
	}

	return playingID;
}

UAkComponent* FAkAudioDevice::SpawnAkComponentAtLocation( class UAkAudioEvent* in_pAkEvent, FVector Location, FRotator Orientation, bool AutoPost, const FString& EventName, bool AutoDestroy, UWorld* in_World)
{
	UAkComponent * AkComponent = NULL;
	if (in_World)
	{
		AkComponent = NewObject<UAkComponent>(in_World->GetWorldSettings());
	}
	else
	{
		AkComponent = NewObject<UAkComponent>();
	}

	if( AkComponent )
	{
		AkComponent->AkAudioEvent = in_pAkEvent;
		AkComponent->EventName = EventName;
		AkComponent->SetWorldLocationAndRotation(Location, Orientation.Quaternion());
		if(in_World)
		{
			AkComponent->RegisterComponentWithWorld(in_World);
		}

		AkComponent->SetAutoDestroy(AutoDestroy);

		if(AutoPost)
		{
			if (AkComponent->PostAssociatedAkEvent(0, FOnAkPostEventCallback()) == AK_INVALID_PLAYING_ID && AutoDestroy)
			{
				AkComponent->ConditionalBeginDestroy();
				AkComponent = NULL;
			}
		}
	}

	return AkComponent;
}

/**
 * Post a trigger to ak soundengine
 *
 * @param in_pszTrigger		Name of the trigger
 * @param in_pAkComponent	AkComponent on which to post the trigger
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::PostTrigger( 
	const TCHAR * in_pszTrigger,
	AActor * in_pActor
	)
{
	AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT;
	AKRESULT eResult = GetGameObjectID( in_pActor, GameObjID );
	if ( m_bSoundEngineInitialized && eResult == AK_Success)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->PostTrigger(TCHAR_TO_AK(in_pszTrigger), GameObjID );
	}
	return eResult;
}

AKRESULT FAkAudioDevice::PostTrigger(
	const UAkTrigger* in_TriggerValue,
	AActor* in_pActor
)
{
	AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT;
	AKRESULT eResult = GetGameObjectID(in_pActor, GameObjID);
	if (m_bSoundEngineInitialized && in_TriggerValue && eResult == AK_Success)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->PostTrigger(in_TriggerValue->TriggerCookedData.TriggerId, GameObjID);
	}
	return eResult;
}

/**
* Set a RTPC in ak soundengine
*
* @param in_pszRtpcName	Name of the RTPC
* @param in_value			Value to set
* @param in_pActor			Actor on which to set the RTPC
* @return Result from ak sound engine
*/
AKRESULT FAkAudioDevice::SetRTPCValue(
	const TCHAR * in_pszRtpcName,
	AkRtpcValue in_value,
	int32 in_interpolationTimeMs = 0,
	AActor * in_pActor = NULL
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		auto RtpcID = SoundEngine->GetIDFromString(TCHAR_TO_AK(in_pszRtpcName));

		eResult = SetRTPCValue(RtpcID, in_value, in_interpolationTimeMs, in_pActor);
	}
	return eResult;
}

/**
 * Set a RTPC in ak soundengine
 *
 * @param in_Rtpc			RTPC Short ID
 * @param in_value			Value to set
 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
 * @param in_pActor			AActor on which to set the RTPC
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetRTPCValue(
	AkRtpcID in_Rtpc,
	AkRtpcValue in_value,
	int32 in_interpolationTimeMs = 0,
	AActor * in_pActor = NULL
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT; // RTPC at global scope is supported
		if (in_pActor)
		{
			eResult = GetGameObjectID(in_pActor, GameObjID);
			if (eResult != AK_Success)
				return eResult;
		}

		eResult = SoundEngine->SetRTPCValue(in_Rtpc, in_value, GameObjID, in_interpolationTimeMs);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetRTPCValue(
	const UAkRtpc* in_RtpcValue,
	AkRtpcValue in_value,
	int32 in_interpolationTimeMs = 0,
	AActor * in_pActor = NULL
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized && in_RtpcValue)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkGameObjectID GameObjID = AK_INVALID_GAME_OBJECT; // RTPC at global scope is supported
		if (in_pActor)
		{
			eResult = GetGameObjectID(in_pActor, GameObjID);
			if (eResult != AK_Success)
				return eResult;
		}

		eResult = SoundEngine->SetRTPCValue(in_RtpcValue->GameParameterCookedData.ShortId, in_value, GameObjID, in_interpolationTimeMs);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetRTPCValueByPlayingID(
	AkRtpcID in_Rtpc,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	int32 in_interpolationTimeMs
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetRTPCValueByPlayingID(in_Rtpc, in_value, in_playingID, in_interpolationTimeMs);
	}
	return eResult;
}

/**
 *  Get the value of a real-time parameter control (by ID)
 *  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.  
 *  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.  
 *  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
 *  @note
 * 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
 * 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.  
 * 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
 * 		
 *  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
 */
AKRESULT FAkAudioDevice::GetRTPCValue(
	const TCHAR * in_pszRtpcName,
	AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
	AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
	AkRtpcValue& out_rValue, 			///< Value returned
	AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->Query.GetRTPCValue(TCHAR_TO_AK(in_pszRtpcName), in_gameObjectID, in_playingID, out_rValue, io_rValueType);
	}
	return eResult;
}

/**
 *  Get the value of a real-time parameter control (by ID)
 *  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.  
 *  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.  
 *  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
 *  @note
 * 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
 * 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.  
 * 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
 * 		
 *  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
 */
AKRESULT FAkAudioDevice::GetRTPCValue(
	AkRtpcID in_Rtpc,
	AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
	AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
	AkRtpcValue& out_rValue, 			///< Value returned
	AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->Query.GetRTPCValue(in_Rtpc, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::GetRTPCValue(
	const UAkRtpc* in_RtpcValue,
	AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
	AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
	AkRtpcValue& out_rValue, 			///< Value returned
	AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized && in_RtpcValue)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->Query.GetRTPCValue(in_RtpcValue->GameParameterCookedData.ShortId, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::ResetRTPCValue(const UAkRtpc* in_RtpcValue, AkGameObjectID in_gameObjectID, int32 in_interpolationTimeMs)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized && in_RtpcValue)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->ResetRTPCValue(in_RtpcValue->GameParameterCookedData.ShortId, in_gameObjectID, in_interpolationTimeMs);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::ResetRTPCValue(AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, int32 in_interpolationTimeMs)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized && in_rtpcID)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->ResetRTPCValue(in_rtpcID, in_gameObjectID, in_interpolationTimeMs);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::ResetRTPCValue(const TCHAR* in_pszRtpcName, AkGameObjectID in_gameObjectID, int32 in_interpolationTimeMs)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->ResetRTPCValue(TCHAR_TO_AK(in_pszRtpcName), in_gameObjectID, in_interpolationTimeMs);
	}
	return eResult;
}

/**
 * Set a state in ak soundengine
 *
 * @param in_pszStateGroup	Name of the state group
 * @param in_pszState		Name of the state
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetState( 
	const TCHAR * in_pszStateGroup,
	const TCHAR * in_pszState
    )
{
	AKRESULT eResult = AK_Success;
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		auto StateGroupID = SoundEngine->GetIDFromString(TCHAR_TO_AK(in_pszStateGroup));
		auto StateID = SoundEngine->GetIDFromString(TCHAR_TO_AK(in_pszState));
		eResult = SoundEngine->SetState(StateGroupID, StateID);
	}
	return eResult;
}

/**
 * Set a state in ak soundengine
 *
 * @param in_StateGroup	State group short ID
 * @param in_State		State short ID
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetState(
	AkStateGroupID in_StateGroup,
	AkStateID in_State
)
{
	AKRESULT eResult = AK_Success;
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetState(in_StateGroup, in_State);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetState(
	const UAkStateValue* in_stateValue
)
{
	AKRESULT eResult = AK_Success;
	if (m_bSoundEngineInitialized && in_stateValue)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetState(in_stateValue->GroupValueCookedData.GroupId, in_stateValue->GroupValueCookedData.Id);
	}
	return eResult;
}

/**
 * Set a switch in ak soundengine
 *
 * @param in_pszSwitchGroup	Name of the switch group
 * @param in_pszSwitchState	Name of the switch
 * @param in_pComponent		AkComponent on which to set the switch
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetSwitch(
	const TCHAR * in_pszSwitchGroup,
	const TCHAR * in_pszSwitchState,
	AActor * in_pActor
	)
{
	AKRESULT eResult = AK_Success;
	if ( m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		auto SwitchGroupID = SoundEngine->GetIDFromString(TCHAR_TO_AK(in_pszSwitchGroup));
		auto SwitchStateID = SoundEngine->GetIDFromString(TCHAR_TO_AK(in_pszSwitchState));
		eResult = SetSwitch(SwitchGroupID, SwitchStateID, in_pActor);
	}
	return eResult;
}

/**
 * Set a switch in ak soundengine
 *
 * @param in_SwitchGroup	Short ID of the switch group
 * @param in_SwitchState	Short ID of the switch
 * @param in_pComponent		AkComponent on which to set the switch
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetSwitch(
	AkSwitchGroupID in_SwitchGroup,
	AkSwitchStateID in_SwitchState,
	AActor * in_pActor
	)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID( in_pActor, GameObjID );
	if ( m_bSoundEngineInitialized && eResult == AK_Success)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetSwitch(in_SwitchGroup, in_SwitchState, GameObjID);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetSwitch(
	const UAkSwitchValue* in_switchValue,
	AActor * in_pActor
)
{
	AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
	// Switches must be bound to a game object. passing DUMMY_GAMEOBJ as default game object.
	AKRESULT eResult = GetGameObjectID(in_pActor, GameObjID);
	if (m_bSoundEngineInitialized && in_switchValue && eResult == AK_Success)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetSwitch(in_switchValue->GroupValueCookedData.GroupId, in_switchValue->GroupValueCookedData.Id, GameObjID);
	}
	return eResult;
}

static AK::SoundEngine::MultiPositionType GetSoundEngineMultiPositionType(AkMultiPositionType in_eType)
{
    switch (in_eType)
    {
    case AkMultiPositionType::SingleSource: return AK::SoundEngine::MultiPositionType_SingleSource;
    case AkMultiPositionType::MultiSources: return AK::SoundEngine::MultiPositionType_MultiSources;
    case AkMultiPositionType::MultiDirections: return AK::SoundEngine::MultiPositionType_MultiDirections;
        // Unknown multi position type!
    default: AKASSERT(false); return AK::SoundEngine::MultiPositionType_SingleSource;
    }
}

/** Sets multiple positions to a single game object.
*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
*  @param in_pGameObjectAkComponent Game Object AkComponent.
*  @param in_pPositions Array of positions to apply.
*  @param in_eMultiPositionType Position type
*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
*
*/
AKRESULT FAkAudioDevice::SetMultiplePositions(
    UAkComponent* in_pGameObjectAkComponent,
    TArray<FTransform> in_aPositions,
    AkMultiPositionType in_eMultiPositionType /*= AkMultiPositionType::MultiDirections*/
)
{
	if (!in_pGameObjectAkComponent)
	{
		return AK_Fail;
	}
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;


	const int numPositions = in_aPositions.Num();
    TArray<AkSoundPosition> aPositions;
    aPositions.Empty();
    for (int i = 0; i < numPositions; ++i)
    {
        AkSoundPosition soundpos;
        FAkAudioDevice::FVectorsToAKWorldTransform(in_aPositions[i].GetLocation(), in_aPositions[i].GetRotation().GetForwardVector(), in_aPositions[i].GetRotation().GetUpVector(), soundpos);
        aPositions.Add(soundpos);
    }
    return SoundEngine->SetMultiplePositions(in_pGameObjectAkComponent->GetAkGameObjectID(), aPositions.GetData(),
                                                 aPositions.Num(), GetSoundEngineMultiPositionType(in_eMultiPositionType));
}

template<typename ChannelConfig>
AKRESULT FAkAudioDevice::SetMultiplePositions(
	UAkComponent* in_pGameObjectAkComponent,
	const TArray<ChannelConfig>& in_aChannelConfigurations,
	const TArray<FTransform>& in_aPositions,
	AkMultiPositionType in_eMultiPositionType /*= AkMultiPositionType::MultiDirections*/
)
{
	if (!in_pGameObjectAkComponent)
	{
		return AK_Fail;
	}
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	const int32 numPositions = FMath::Min(in_aPositions.Num(), in_aChannelConfigurations.Num());

	TArray<AkChannelEmitter> emitters;
	emitters.Reserve(numPositions);
	for (int i = 0; i < numPositions; ++i)
	{
		AkSoundPosition soundpos;
		FAkAudioDevice::FVectorsToAKWorldTransform(in_aPositions[i].GetLocation(), in_aPositions[i].GetRotation().GetForwardVector(), in_aPositions[i].GetRotation().GetUpVector(), soundpos);

		AkChannelConfig config;
		GetChannelConfig(in_aChannelConfigurations[i], config);

		emitters.Add(AkChannelEmitter());
		emitters[i].uInputChannels = config.uChannelMask;
		emitters[i].position = soundpos;
	}

	return SoundEngine->SetMultiplePositions(in_pGameObjectAkComponent->GetAkGameObjectID(), emitters.GetData(),
		emitters.Num(), GetSoundEngineMultiPositionType(in_eMultiPositionType));
}

AKRESULT FAkAudioDevice::SetMultiplePositions(
	UAkComponent* in_pGameObjectAkComponent,
	const TArray<AkChannelConfiguration>& in_aChannelConfigurations,
	const TArray<FTransform>& in_aPositions,
	AkMultiPositionType in_eMultiPositionType
)
{
	return SetMultiplePositions<AkChannelConfiguration>(in_pGameObjectAkComponent, in_aChannelConfigurations, in_aPositions, in_eMultiPositionType);
}

AKRESULT FAkAudioDevice::SetMultiplePositions(
	UAkComponent* in_pGameObjectAkComponent,
	const TArray<FAkChannelMask>& in_channelMasks,
	const TArray<FTransform>& in_aPositions,
	AkMultiPositionType in_eMultiPositionType
)
{
	return SetMultiplePositions<FAkChannelMask>(in_pGameObjectAkComponent, in_channelMasks, in_aPositions, in_eMultiPositionType);
}

/** Sets multiple positions to a single game object.
*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
*  @param in_GameObjectID Game Object identifier.
*  @param in_pPositions Array of positions to apply.
*  @param in_NumPositions Number of positions specified in the provided array.
*  @param in_eMultiPositionType Position type
*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
*/
AKRESULT FAkAudioDevice::SetMultiplePositions(
    AkGameObjectID in_GameObjectID,
    const AkSoundPosition * in_pPositions,
    AkUInt16 in_NumPositions,
    AK::SoundEngine::MultiPositionType in_eMultiPositionType /*= AK::SoundEngine::MultiDirections*/
    )
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType);
}

/** Sets multiple positions to a single game object, with flexible assignment of input channels.
*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
*  @param in_GameObjectID Game Object identifier.
*  @param in_pPositions Array of positions to apply.
*  @param in_NumPositions Number of positions specified in the provided array.
*  @param in_eMultiPositionType Position type
*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
*/
AKRESULT FAkAudioDevice::SetMultiplePositions(
    AkGameObjectID in_GameObjectID,
    const AkChannelEmitter * in_pPositions,
    AkUInt16 in_NumPositions,
    AK::SoundEngine::MultiPositionType in_eMultiPositionType /*= AK::SoundEngine::MultiDirections*/
    )
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return SoundEngine->SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType);
}

/**
 * Set auxiliary sends
 *
 * @param in_GameObjId		Wwise Game Object ID
 * @param in_AuxSendValues	Array of AkAuxSendValue, containins all Aux Sends to set on the game objectt
 * @return Result from ak sound engine
 */
AKRESULT FAkAudioDevice::SetAuxSends(
	const UAkComponent* in_akComponent,
	TArray<AkAuxSendValue>& in_AuxSendValues
	)
{
	AKRESULT eResult = AK_Success;
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetGameObjectAuxSendValues(in_akComponent->GetAkGameObjectID(), in_AuxSendValues.GetData(), in_AuxSendValues.Num());
	}
	
	return eResult;
}

void FAkAudioDevice::GetChannelConfig(AkChannelConfiguration ChannelConfiguration, AkChannelConfig& config)
{
	switch (ChannelConfiguration)
	{
	case AkChannelConfiguration::Ak_MainMix:
		config.eConfigType = AK_ChannelConfigType_UseDeviceMain;
		break;
	case AkChannelConfiguration::Ak_Passthrough:
		config.eConfigType = AK_ChannelConfigType_UseDevicePassthrough;
		break;
	case AkChannelConfiguration::Ak_LFE:
		config.SetStandard(AK_SPEAKER_SETUP_0POINT1);
		break;
	case AkChannelConfiguration::AK_Audio_Objects:
		config.SetObject();
		break;
	case AkChannelConfiguration::Ak_1_0:
		config.SetStandard(AK_SPEAKER_SETUP_MONO);
		break;
	case AkChannelConfiguration::Ak_2_0:
		config.SetStandard(AK_SPEAKER_SETUP_STEREO);
		break;
	case AkChannelConfiguration::Ak_2_1:
		config.SetStandard(AK_SPEAKER_SETUP_2POINT1);
		break;
	case AkChannelConfiguration::Ak_3_0:
		config.SetStandard(AK_SPEAKER_SETUP_3STEREO);
		break;
	case AkChannelConfiguration::Ak_3_1:
		config.SetStandard(AK_SPEAKER_SETUP_3POINT1);
		break;
	case AkChannelConfiguration::Ak_4_0:
		config.SetStandard(AK_SPEAKER_SETUP_4);
		break;
	case AkChannelConfiguration::Ak_4_1:
		config.SetStandard(AK_SPEAKER_SETUP_4POINT1);
		break;
	case AkChannelConfiguration::Ak_5_0:
		config.SetStandard(AK_SPEAKER_SETUP_5);
		break;
	case AkChannelConfiguration::Ak_5_1:
		config.SetStandard(AK_SPEAKER_SETUP_5POINT1);
		break;
	case AkChannelConfiguration::Ak_7_1:
		config.SetStandard(AK_SPEAKER_SETUP_7POINT1);
		break;
	case AkChannelConfiguration::Ak_5_1_2:
		config.SetStandard(AK_SPEAKER_SETUP_DOLBY_5_1_2);
		break;
	case AkChannelConfiguration::Ak_7_1_2:
		config.SetStandard(AK_SPEAKER_SETUP_DOLBY_7_1_2);
		break;
	case AkChannelConfiguration::Ak_7_1_4:
		config.SetStandard(AK_SPEAKER_SETUP_DOLBY_7_1_4);
		break;
	case AkChannelConfiguration::Ak_Auro_9_1:
		config.SetStandard(AK_SPEAKER_SETUP_AURO_9POINT1);
		break;
	case AkChannelConfiguration::Ak_Auro_10_1:
		config.SetStandard(AK_SPEAKER_SETUP_AURO_10POINT1);
		break;
	case AkChannelConfiguration::Ak_Auro_11_1:
		config.SetStandard(AK_SPEAKER_SETUP_AURO_11POINT1);
		break;
	case AkChannelConfiguration::Ak_Auro_13_1:
		config.SetStandard(AK_SPEAKER_SETUP_AURO_13POINT1_751);
		break;
	case AkChannelConfiguration::Ak_Ambisonics_1st_order:
		config.SetAmbisonic(4);
		break;
	case AkChannelConfiguration::Ak_Ambisonics_2nd_order:
		config.SetAmbisonic(9);
		break;
	case AkChannelConfiguration::Ak_Ambisonics_3rd_order:
		config.SetAmbisonic(16);
		break;
	case AkChannelConfiguration::Ak_Ambisonics_4th_order:
		config.SetAmbisonic(25);
		break;
	case AkChannelConfiguration::Ak_Ambisonics_5th_order:
		config.SetAmbisonic(36);
		break;

	case AkChannelConfiguration::Ak_Parent:
	default:
		config.Clear();
		break;
	}
}

void FAkAudioDevice::GetChannelConfig(FAkChannelMask SpeakerConfig, AkChannelConfig& config)
{
	config.SetStandard(SpeakerConfig.ChannelMask);
}

/**
* Set spatial audio room
*
* @param in_GameObjId		Wwise Game Object ID
* @param in_RoomID	ID of the room that the game object is inside.
* @return Result from ak sound engine
*/
AKRESULT FAkAudioDevice::SetInSpatialAudioRoom(
	const AkGameObjectID in_GameObjId,
	AkRoomID in_RoomID
)
{
	AKRESULT eResult = AK_Success;
#ifdef AK_ENABLE_ROOMS
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->SetGameObjectInRoom(in_GameObjId, in_RoomID);
	}
#endif
	return eResult;
}

AKRESULT FAkAudioDevice::SetBusConfig(
	const FString&	in_BusName,
	AkChannelConfig	in_Config
	)
{
	AKRESULT eResult = AK_Fail;
	if (in_BusName.IsEmpty())
	{
		return eResult;
	}

	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkUniqueID BusId = GetShortIDFromString(in_BusName);
		eResult = SoundEngine->SetBusConfig(BusId, in_Config);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetPanningRule(
	AkPanningRule		in_ePanningRule
	)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetPanningRule(in_ePanningRule);
	}

	return eResult;
}

AkOutputDeviceID FAkAudioDevice::GetOutputID(
	const FString& in_szShareSet,
	AkUInt32 in_idDevice
	)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_INVALID_OUTPUT_DEVICE_ID;

	return SoundEngine->GetOutputID(TCHAR_TO_AK(*in_szShareSet), in_idDevice);
}

AKRESULT FAkAudioDevice::ReplaceMainOutput(const AkOutputSettings& MainOutputSettings)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	AKRESULT Result = SoundEngine->ReplaceOutput(MainOutputSettings, 0);
	SoundEngine->RenderAudio();
	return Result;
}

AKRESULT FAkAudioDevice::GetSpeakerAngles(
	TArray<AkReal32>& out_pfSpeakerAngles,
	AkReal32& out_fHeightAngle,
	AkOutputDeviceID in_idOutput
	)
{
	AKRESULT eResult = AK_Fail;

	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkUInt32 numSpeakers;

		// Retrieve the number of speaker and height angle
		eResult = SoundEngine->GetSpeakerAngles(NULL, numSpeakers, out_fHeightAngle);
		if (eResult != AK_Success)
			return eResult;

		// Retrieve the speaker angles
		out_pfSpeakerAngles.SetNum(numSpeakers);
		eResult = SoundEngine->GetSpeakerAngles(out_pfSpeakerAngles.GetData(), numSpeakers, out_fHeightAngle, in_idOutput);
	}
	
	return eResult;
}

AKRESULT FAkAudioDevice::SetSpeakerAngles(
	const TArray<AkReal32>& in_pfSpeakerAngles,
	AkReal32 in_fHeightAngle,
	AkOutputDeviceID in_idOutput
	)
{
	AKRESULT eResult = AK_Fail;

	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetSpeakerAngles(in_pfSpeakerAngles.GetData(), in_pfSpeakerAngles.Num(), in_fHeightAngle, in_idOutput);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetGameObjectOutputBusVolume(
	const UAkComponent* in_pEmitter,
	const UAkComponent* in_pListener,
	float in_fControlValue	
	)
{
	AKRESULT eResult = AK_Success;

	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		const AkGameObjectID emitterId = in_pEmitter ? in_pEmitter->GetAkGameObjectID() : DUMMY_GAMEOBJ;
		const AkGameObjectID listenerId = in_pListener ? in_pListener->GetAkGameObjectID() : DUMMY_GAMEOBJ;
		eResult = SoundEngine->SetGameObjectOutputBusVolume(emitterId, listenerId, in_fControlValue);
	}

	return eResult;
}

/**
 * Obtain a pointer to the singleton instance of FAkAudioDevice
 *
 * @return Pointer to the singleton instance of FAkAudioDevice
 */
FAkAudioDevice * FAkAudioDevice::Get()
{
	if (UNLIKELY(m_EngineExiting))
	{
		return nullptr;
	}

	if (LIKELY(FAkAudioModule::AkAudioModuleInstance != nullptr))
	{
		return FAkAudioModule::AkAudioModuleInstance->GetAkAudioDevice();
	}
	else
	{
		FAkAudioModule* mod = FModuleManager::LoadModulePtr<FAkAudioModule>(TEXT("AkAudio"));
		ensure(FAkAudioModule::AkAudioModuleInstance == mod);
		return mod ? mod->GetAkAudioDevice() : nullptr;
	}
}

/**
 * Gets the system sample rate
 *
 * @return Sample rate
 */
AkUInt32 FAkAudioDevice::GetSampleRate()
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return 0;

	return m_bSoundEngineInitialized ? SoundEngine->GetSampleRate() : 0;
}

/**
 * Enables/disables offline rendering
 *
 * @param bEnable		Set to true to enable offline rendering
 */
AKRESULT FAkAudioDevice::SetOfflineRendering(bool bEnable)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return m_bSoundEngineInitialized ? SoundEngine->SetOfflineRendering(bEnable) : AK_Fail;
}

/**
 * Sets the offline rendering frame time in seconds.
 *
 * @param FrameTimeInSeconds		Frame time in seconds used during offline rendering
 */
AKRESULT FAkAudioDevice::SetOfflineRenderingFrameTime(AkReal32 FrameTimeInSeconds)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return m_bSoundEngineInitialized ? SoundEngine->SetOfflineRenderingFrameTime(FrameTimeInSeconds) : AK_Fail;
}

/**
 * Registers a callback used for retrieving audio samples.
 *
 * @param Callback		Capture callback function to register
 * @param OutputId			The audio device specific id, return by AK::SoundEngine::AddOutput or AK::SoundEngine::GetOutputID
 * @param Cookie			Callback cookie that will be sent to the callback function along with additional information
 */
AKRESULT FAkAudioDevice::RegisterCaptureCallback(AkCaptureCallbackFunc Callback, AkOutputDeviceID OutputId /*= AK_INVALID_OUTPUT_DEVICE_ID*/, void* Cookie /*= nullptr*/)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return m_bSoundEngineInitialized ? SoundEngine->RegisterCaptureCallback(Callback, OutputId, Cookie) : AK_Fail;
}

/**
 * Unregisters a callback used for retrieving audio samples.
 *
 * @param Callback		Capture callback function to register
 * @param OutputId			The audio device specific id, return by AK::SoundEngine::AddOutput or AK::SoundEngine::GetOutputID
 * @param Cookie			Callback cookie that will be sent to the callback function along with additional information
 */
AKRESULT FAkAudioDevice::UnregisterCaptureCallback(AkCaptureCallbackFunc Callback, AkOutputDeviceID OutputId /*= AK_INVALID_OUTPUT_DEVICE_ID*/, void* Cookie /*= nullptr*/)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	return m_bSoundEngineInitialized ? SoundEngine->UnregisterCaptureCallback(Callback, OutputId, Cookie) : AK_Fail;
}

/**
 * Stop all audio associated with a game object
 *
 * @param in_GameObjID		ID of the game object
 */
void FAkAudioDevice::StopGameObject( UAkComponent * in_pComponent )
{
	AkGameObjectID gameObjId = DUMMY_GAMEOBJ;
	if ( in_pComponent )
	{
		gameObjId = in_pComponent->GetAkGameObjectID();
	}
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StopAll( gameObjId );
	}
}

/**
 * Stop all audio associated with a playing ID
 *
 * @param in_playingID		Playing ID to stop
 * @param in_uTransitionDuration Fade duration
 * @param in_eFadeCurve          Curve type to be used for the transition
 */
void FAkAudioDevice::StopPlayingID( AkPlayingID in_playingID, 
                                    AkTimeMs in_uTransitionDuration /*= 0*/, 
                                    AkCurveInterpolation in_eFadeCurve /*= AkCurveInterpolation_Linear*/)
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Stop, in_playingID, in_uTransitionDuration, in_eFadeCurve );
	}
}

/**
 * Register an ak audio component with ak sound engine
 *
 * @param in_pComponent		Pointer to the component to register
 */
void FAkAudioDevice::RegisterComponent( UAkComponent * in_pComponent )
{
	if (m_bSoundEngineInitialized && in_pComponent)
	{
		if (in_pComponent->UseDefaultListeners())
			m_defaultEmitters.Add(in_pComponent);

		FString WwiseGameObjectName = TEXT("");
		in_pComponent->GetAkGameObjectName(WwiseGameObjectName);
		
		const AkGameObjectID gameObjId = in_pComponent->GetAkGameObjectID();
		FAkAudioDevice_Helpers::RegisterGameObject(gameObjId, WwiseGameObjectName);

		if (CallbackManager != nullptr)
			CallbackManager->RegisterGameObject(gameObjId);
	}
}

/**
 * Register a game object with ak sound engine
 *
 * @param GameObjectID		ID of the game object to register
 */
void FAkAudioDevice::RegisterComponent(AkGameObjectID GameObjectID)
{
	if (m_bSoundEngineInitialized && GameObjectID)
	{
		FAkAudioDevice_Helpers::RegisterGameObject(GameObjectID, "");

		if (CallbackManager != nullptr)
			CallbackManager->RegisterGameObject(GameObjectID);
	}
}

/**
 * Unregister an ak audio component with ak sound engine
 *
 * @param in_pComponent		Pointer to the component to unregister
 */
void FAkAudioDevice::UnregisterComponent( UAkComponent * in_pComponent )
{
	if (m_bSoundEngineInitialized && in_pComponent)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (LIKELY(SoundEngine))
		{
			const AkGameObjectID gameObjId = in_pComponent->GetAkGameObjectID();
			SoundEngine->UnregisterGameObj(gameObjId);

			if (CallbackManager != nullptr)
			{
				CallbackManager->UnregisterGameObject(gameObjId);
			}
		}
	}

	if (m_defaultListeners.Contains(in_pComponent))
	{
		RemoveDefaultListener(in_pComponent);
	}

	if (in_pComponent->UseDefaultListeners())
	{
		m_defaultEmitters.Remove(in_pComponent);
	}

	check(!m_defaultListeners.Contains(in_pComponent) && !m_defaultEmitters.Contains(in_pComponent));

	if (m_SpatialAudioListener == in_pComponent)
		m_SpatialAudioListener = nullptr;
}

void FAkAudioDevice::UnregisterComponent( AkGameObjectID GameObjectId )
{
	if (m_bSoundEngineInitialized && GameObjectId)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (LIKELY(SoundEngine))
		{
			SoundEngine->UnregisterGameObj(GameObjectId);
		}

		if (CallbackManager != nullptr)
		{
			CallbackManager->UnregisterGameObject(GameObjectId);
		}
	}
}

AKRESULT FAkAudioDevice::SetGeometry(AkGeometrySetID GeometrySetID, const AkGeometryParams& Params)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->SetGeometry(GeometrySetID, Params);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetGeometryInstance(AkGeometryInstanceID GeometryInstanceID, const AkGeometryInstanceParams& Params)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->SetGeometryInstance(GeometryInstanceID, Params);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::RemoveGeometrySet(AkGeometrySetID GeometrySetID)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->RemoveGeometry(GeometrySetID);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::RemoveGeometryInstance(AkGeometryInstanceID GeometryInstanceID)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->RemoveGeometryInstance(GeometryInstanceID);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetEarlyReflectionsAuxBus(UAkComponent* in_pComponent, const AkUInt32 AuxBusID)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && in_pComponent)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		const AkGameObjectID gameObjId = in_pComponent->GetAkGameObjectID();
		eResult = SpatialAudio->SetEarlyReflectionsAuxSend(gameObjId, AuxBusID);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetEarlyReflectionsVolume(UAkComponent* in_pComponent, float in_fSendVolume)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && in_pComponent)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		const AkGameObjectID gameObjId = in_pComponent->GetAkGameObjectID();
		eResult = SpatialAudio->SetEarlyReflectionsVolume(gameObjId, in_fSendVolume);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetReflectionsOrder(int Order, bool RefreshPaths)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		eResult = SpatialAudio->SetReflectionsOrder(Order, RefreshPaths);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetMultipleObstructionAndOcclusion(AkGameObjectID in_Object, AkGameObjectID in_listener, AkObstructionOcclusionValues* ObstructionOcclusionValues, AkUInt32 in_uNumOcclusionObstruction)
{
	AKRESULT eResult = AK_Fail;
	if(m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;
		eResult = SoundEngine->SetMultipleObstructionAndOcclusion(in_Object, in_listener, ObstructionOcclusionValues, in_uNumOcclusionObstruction);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetOcclusionAndObstruction(AkGameObjectID in_Object, AkGameObjectID in_listener, AkReal32 Obstruction, AkReal32 Occlusion)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;
		eResult = SoundEngine->SetObjectObstructionAndOcclusion(in_Object, in_listener, Obstruction, Occlusion);
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetPortalObstructionAndOcclusion(UAkPortalComponent* in_pPortal, float in_fObstructionValue, float in_fOcclusionValue)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && in_pPortal)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		const AkPortalID portalID = in_pPortal->GetPortalID();
		eResult = SpatialAudio->SetPortalObstructionAndOcclusion(portalID, in_fObstructionValue, in_fOcclusionValue);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetGameObjectToPortalObstruction(UAkComponent* in_pComponent, UAkPortalComponent* in_pPortal, float in_fObstructionValue)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && in_pComponent && in_pPortal)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		const AkGameObjectID gameObjId = in_pComponent->GetAkGameObjectID();
		const AkPortalID portalID = in_pPortal->GetPortalID();
		eResult = SpatialAudio->SetGameObjectToPortalObstruction(gameObjId, portalID, in_fObstructionValue);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetPortalToPortalObstruction(UAkPortalComponent* in_pPortal0, UAkPortalComponent* in_pPortal1, float in_fObstructionValue)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && in_pPortal0 && in_pPortal1)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		const AkPortalID portalID0 = in_pPortal0->GetPortalID();
		const AkPortalID portalID1 = in_pPortal1->GetPortalID();
		eResult = SpatialAudio->SetPortalToPortalObstruction(portalID0, portalID1, in_fObstructionValue);
	}
	return eResult;
}

void FAkAudioDevice::UpdateDefaultActiveListeners()
{
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		auto NumDefaultListeners = m_defaultListeners.Num();
		auto pListenerIds = (AkGameObjectID*)alloca(NumDefaultListeners * sizeof(AkGameObjectID));
		int index = 0;
		for (auto DefaultListenerIter = m_defaultListeners.CreateConstIterator(); DefaultListenerIter; ++DefaultListenerIter)
			pListenerIds[index++] = (*DefaultListenerIter)->GetAkGameObjectID();

		if (NumDefaultListeners > 0)
		{
			SoundEngine->SetDefaultListeners(pListenerIds, NumDefaultListeners);
		}
	}
}

AKRESULT FAkAudioDevice::SetPosition(UAkComponent* in_akComponent, const AkSoundPosition& in_SoundPosition)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		return SoundEngine->SetPosition(in_akComponent->GetAkGameObjectID(), in_SoundPosition);
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::AddRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams)
{
	if (ShouldNotifySoundEngine(in_pRoom->GetWorld()->WorldType))
	{
		AKRESULT result = AK_Fail;
		if (m_bSoundEngineInitialized)
		{
			auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
			if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

			result = SpatialAudio->SetRoom(in_pRoom->GetRoomID(), in_RoomParams, TCHAR_TO_ANSI(*in_pRoom->GetRoomName()));
			if (result == AK_Success)
			{
				IndexRoom(in_pRoom);
				UpdateRoomsForPortals(in_pRoom->GetWorld());
			}
		}
		return result;
	}

	IndexRoom(in_pRoom);
	UpdateRoomsForPortals(in_pRoom->GetWorld());
	return AK_Success;
}

AKRESULT FAkAudioDevice::UpdateRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams)
{
	if (ShouldNotifySoundEngine(in_pRoom->GetWorld()->WorldType))
	{
		AKRESULT result = AK_Fail;
		if (m_bSoundEngineInitialized)
		{
			check(in_pRoom->HasBeenRegisteredWithWwise());
			auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
			if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

			check(in_pRoom->HasBeenRegisteredWithWwise());

			result = SpatialAudio->SetRoom(in_pRoom->GetRoomID(), in_RoomParams, TCHAR_TO_ANSI(*in_pRoom->GetRoomName()));
			if (result == AK_Success)
				UpdateRoomsForPortals(in_pRoom->GetWorld());
		}
		return result;
	}

	UpdateRoomsForPortals(in_pRoom->GetWorld());
	return AK_Success;
}

AKRESULT FAkAudioDevice::RemoveRoom(UAkRoomComponent* in_pRoom)
{
	if (ShouldNotifySoundEngine(in_pRoom->GetWorld()->WorldType))
	{
		AKRESULT result = AK_Fail;
		if (m_bSoundEngineInitialized)
		{
			auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
			if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

			result = SpatialAudio->RemoveRoom(in_pRoom->GetRoomID());
			if (result == AK_Success)
			{
				UnindexRoom(in_pRoom);
				UpdateRoomsForPortals(in_pRoom->GetWorld());
			}
		}

		return result;
	}

	UnindexRoom(in_pRoom);
	UpdateRoomsForPortals(in_pRoom->GetWorld());
	return AK_Success;
}

AKRESULT FAkAudioDevice::SetGameObjectRadius(UAkComponent* in_akComponent, float in_outerRadius, float in_innerRadius)
{
	if (!m_bSoundEngineInitialized)
		return AK_Fail;

	auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
	if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

	return SpatialAudio->SetGameObjectRadius(AkGameObjectID(in_akComponent), in_outerRadius, in_innerRadius);
}

AKRESULT FAkAudioDevice::SetImageSource(AAkSpotReflector* in_pSpotReflector, const AkImageSourceSettings& in_ImageSourceInfo, AkUniqueID in_AuxBusID, UAkComponent* in_AkComponent)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		return SpatialAudio->SetImageSource(in_pSpotReflector->GetImageSourceID(), in_ImageSourceInfo, TCHAR_TO_ANSI(*in_pSpotReflector->GetName()), in_AuxBusID, in_AkComponent->GetAkGameObjectID());
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::RemoveImageSource(AAkSpotReflector* in_pSpotReflector, AkUniqueID in_AuxBusID, UAkComponent* in_AkComponent)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		return SpatialAudio->RemoveImageSource(in_pSpotReflector->GetImageSourceID(), in_AuxBusID, in_AkComponent->GetAkGameObjectID());
	}

	return AK_Fail;
}

AKRESULT FAkAudioDevice::ClearImageSources(AkUniqueID in_AuxBusID, UAkComponent* in_AkComponent)
{
	if (m_bSoundEngineInitialized)
	{
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (UNLIKELY(!SpatialAudio)) return AK_NotInitialized;

		return SpatialAudio->ClearImageSources(in_AuxBusID, in_AkComponent == NULL ? AK_INVALID_GAME_OBJECT : in_AkComponent->GetAkGameObjectID());
	}

	return AK_Fail;
}

void FAkAudioDevice::SetListeners(UAkComponent* in_pEmitter, const TArray<UAkComponent*>& in_listenerSet)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	check(!in_pEmitter->UseDefaultListeners());

	m_defaultEmitters.Remove(in_pEmitter); //This emitter is no longer using the default listener set.

	auto NumListeners = in_listenerSet.Num();
	auto pListenerIds = (AkGameObjectID*)alloca(NumListeners * sizeof(AkGameObjectID));
	int index = 0;
	for (const auto& Listener : in_listenerSet)
		pListenerIds[index++] = Listener->GetAkGameObjectID();

	SoundEngine->SetListeners(in_pEmitter->GetAkGameObjectID(), pListenerIds, NumListeners);
}

bool FAkAudioDevice::SetSpatialAudioListener(UAkComponent* in_pListener)
{
#if WITH_EDITOR
	if (in_pListener == EditorListener)
	{
		return false;
	}
#endif
	m_SpatialAudioListener = in_pListener;

	auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
	if (UNLIKELY(!SpatialAudio)) return false;

	SpatialAudio->RegisterListener((AkGameObjectID)m_SpatialAudioListener);
	return true;
}

UAkComponent* FAkAudioDevice::GetSpatialAudioListener() const
{
	return m_SpatialAudioListener;
}

UAkComponent* FAkAudioDevice::GetAkComponent(class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType)
{
	bool ComponentCreated;
	return GetAkComponent(AttachToComponent, AttachPointName, Location, LocationType, ComponentCreated);
}

UAkComponent* FAkAudioDevice::GetAkComponent( class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType, bool& ComponentCreated )
{
	if (!AttachToComponent)
	{
		return NULL;
	}

	UAkComponent* AkComponent = NULL;
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules::KeepRelativeTransform;

	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return nullptr;

	if( GEngine && SoundEngine->IsInitialized())
	{
		AActor * Actor = AttachToComponent->GetOwner();
		if( Actor ) 
		{
			if( !IsValid(Actor) )
			{
				// Avoid creating component if we're trying to play a sound on an already destroyed actor.
				return NULL;
			}

			TArray<UAkComponent*> AkComponents;
			Actor->GetComponents(AkComponents);
			for ( int32 CompIdx = 0; CompIdx < AkComponents.Num(); CompIdx++ )
			{
				UAkComponent* pCompI = AkComponents[CompIdx];
				if ( pCompI && pCompI->IsRegistered() )
				{
					if ( AttachToComponent == pCompI )
					{
						return pCompI;
					}

					if ( AttachToComponent != pCompI->GetAttachParent() 
						|| AttachPointName != pCompI->GetAttachSocketName() )
					{
						continue;
					}

					// If a location is requested, try to match location.
					if ( Location )
					{
						if (LocationType == EAttachLocation::KeepWorldPosition)
						{
							AttachRules = FAttachmentTransformRules::KeepWorldTransform;
							if ( !FVector::PointsAreSame(*Location, pCompI->GetComponentLocation()) )
								continue;
						}
						else
						{
							AttachRules = FAttachmentTransformRules::KeepRelativeTransform;
							auto RelLoc = pCompI->GetRelativeLocation();
							if ( !FVector::PointsAreSame(*Location, RelLoc) )
								continue;
						}
					}

					// AkComponent found which exactly matches the attachment: reuse it.
					ComponentCreated = false;
					return pCompI;
				}
			}
		}
		else
		{
			// Try to find if there is an AkComponent attached to AttachToComponent (will be the case if AttachToComponent has no owner)
			const TArray<USceneComponent*> AttachChildren = AttachToComponent->GetAttachChildren();
			for(int32 CompIdx = 0; CompIdx < AttachChildren.Num(); CompIdx++)
			{
				UAkComponent* pCompI = Cast<UAkComponent>(AttachChildren[CompIdx]);
				if ( pCompI && pCompI->IsRegistered() )
				{
					// There is an associated AkComponent to AttachToComponent, no need to add another one.
					ComponentCreated = false;
					return pCompI;
				}
			}
		}

		if ( AkComponent == NULL )
		{
			if( Actor )
			{
				AkComponent = NewObject<UAkComponent>(Actor);
			}
			else
			{
				AkComponent = NewObject<UAkComponent>();
			}
		}

		ComponentCreated = true;
		check( AkComponent );

		if (Location)
		{
			if (LocationType == EAttachLocation::KeepWorldPosition)
			{
				AttachRules = FAttachmentTransformRules::KeepWorldTransform;
				AkComponent->SetWorldLocation(*Location);
			}
			else
			{
				AttachRules = FAttachmentTransformRules::KeepRelativeTransform;
				AkComponent->SetRelativeLocation(*Location);
			}
		}

		AkComponent->RegisterComponentWithWorld(AttachToComponent->GetWorld());
		AkComponent->AttachToComponent(AttachToComponent, AttachRules, AttachPointName);
	}

	return( AkComponent );
}


/**
* Cancel the callback cookie for a dispatched event
*
* @param in_cookie			The cookie to cancel
*/
void FAkAudioDevice::CancelEventCallbackCookie(void* in_cookie)
{
	if (m_bSoundEngineInitialized)
	{
		CallbackManager->CancelEventCallback(in_cookie);
	}
}

/**
* Cancel the callback cookie for a dispatched event
*
* @param in_cookie			The cookie to cancel
*/
void FAkAudioDevice::CancelEventCallbackDelegate(const FOnAkPostEventCallback& in_Delegate)
{
	if (m_bSoundEngineInitialized)
	{
		CallbackManager->CancelEventCallback(in_Delegate);
	}
}

AKRESULT FAkAudioDevice::SetAttenuationScalingFactor(AActor* Actor, float ScalingFactor)
{
	AKRESULT eResult = AK_Fail;
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		AkGameObjectID GameObjID = DUMMY_GAMEOBJ;
		eResult = GetGameObjectID( Actor, GameObjID );
		if( eResult == AK_Success )
		{
			eResult = SoundEngine->SetScalingFactor(GameObjID, ScalingFactor);
		}
	}

	return eResult;
}

AKRESULT FAkAudioDevice::SetAttenuationScalingFactor(UAkComponent* AkComponent, float ScalingFactor)
{
	AKRESULT eResult = AK_Fail;
	if ( m_bSoundEngineInitialized && AkComponent)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetScalingFactor(AkComponent->GetAkGameObjectID(), ScalingFactor);
	}
	return eResult;
}

AKRESULT FAkAudioDevice::SetDistanceProbe(UAkComponent* Listener, UAkComponent* DistanceProbe)
{
	AKRESULT eResult = AK_Fail;
	if (m_bSoundEngineInitialized && Listener)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

		eResult = SoundEngine->SetDistanceProbe(Listener->GetAkGameObjectID(), DistanceProbe != nullptr ? DistanceProbe->GetAkGameObjectID() : AK_INVALID_GAME_OBJECT );
	}
	return eResult;
}

#if WITH_EDITOR
#ifndef AK_OPTIMIZED
AkUnrealErrorTranslator FAkAudioDevice::m_UnrealErrorTranslator;
#endif //AK_OPTIMIZED
#endif //WITH_EDITOR

bool FAkAudioDevice::EnsureInitialized()
{
	static bool bPermanentInitializationFailure = false;
	static bool bLogWwiseVersionOnce = true;

	if (LIKELY(m_bSoundEngineInitialized))
	{
		return true;
	}
	if (UNLIKELY(bPermanentInitializationFailure))
	{
		return false;
	}

	UE_CLOG(bLogWwiseVersionOnce, LogAkAudio, Log,
		TEXT("Wwise(R) SDK Version %d.%d.%d Build %d. Copyright (c) 2006-%d Audiokinetic Inc."),
		AK_WWISESDK_VERSION_MAJOR,
		AK_WWISESDK_VERSION_MINOR,
		AK_WWISESDK_VERSION_SUBMINOR,
		AK_WWISESDK_VERSION_BUILD,
		AK_WWISESDK_VERSION_MAJOR);
	bLogWwiseVersionOnce = false;

	// We don't want sound in those cases.
	if (FParse::Param(FCommandLine::Get(), TEXT("nosound")))
	{
		UE_LOG(LogAkAudio, Display, TEXT("Wwise Sound Engine is disabled: \"nosound\" command line parameter."));
		bPermanentInitializationFailure = true;
		return false;
	}
	if (FApp::IsBenchmarking())
	{
		UE_LOG(LogAkAudio, Display, TEXT("Wwise Sound Engine is disabled: App is benchmarking."));
		bPermanentInitializationFailure = true;
		return false;
	}
	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogAkAudio, Display, TEXT("Wwise Sound Engine is disabled: Running a dedicated server."));
		bPermanentInitializationFailure = true;
		return false;
	}
	if (IsRunningCommandlet())
	{
		UE_LOG(LogAkAudio, Display, TEXT("Wwise Sound Engine is disabled: Running a commandlet."));
		bPermanentInitializationFailure = true;
		return false;
	}

	auto* FileHandlerModule = IWwiseFileHandlerModule::GetModule();
	if (UNLIKELY(!FileHandlerModule))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Wwise Initialization Error: No file handling module"));
		bPermanentInitializationFailure = true;
		return false;
	}
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Wwise Initialization Error: No Sound Engine"));
		bPermanentInitializationFailure = true;
		return false;
	}

	IOHook = FileHandlerModule->InstantiateIOHook();
	if (UNLIKELY(!IOHook))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Wwise Initialization Error: No IO Hook"));
		bPermanentInitializationFailure = true;
		return false;
	}

	// From this point on, if we get an error, we can try initializing later
	if (UNLIKELY(!FAkSoundEngineInitialization::Initialize(IOHook)))
	{
		UE_LOG(LogAkAudio, Display, TEXT("Wwise Initialization Error."));
		FAkSoundEngineInitialization::Finalize(IOHook);
		delete IOHook;
		IOHook = nullptr;
		return false;
	}

	UE_LOG(LogAkAudio, Log, TEXT("Wwise Sound Engine successfully initialized."));

	SetLocalOutput();

	// Init dummy game object
	SoundEngine->RegisterGameObj(DUMMY_GAMEOBJ, "Unreal Global");

#if WITH_EDITOR
	if (!IsRunningGame())
	{
		AkGameObjectID tempID = DUMMY_GAMEOBJ;
		SoundEngine->SetListeners(DUMMY_GAMEOBJ, &tempID, 1);
	}
#endif

	m_bSoundEngineInitialized = true;

	CallbackInfoPool = new AkCallbackInfoPool;
	// Go get the max number of Aux busses
	MaxAuxBus = AK_MAX_AUX_PER_OBJ;
	if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
	{
		MaxAuxBus = AkSettings->MaxSimultaneousReverbVolumes;
	}

	//TUniquePtr
	CallbackManager = new FAkComponentCallbackManager();
	SetCurrentAudioCulture(GetDefaultLanguage());

	UE_LOG(LogAkAudio, Log, TEXT("Initialization complete."));

	return CallbackManager != nullptr;
}

void FAkAudioDevice::SetLocalOutput()
{
	auto* Monitor = FWwiseLowLevelMonitor::Get();
	if (UNLIKELY(!Monitor))
	{
		return;
	}

	Monitor->ResetTranslator();
#if WITH_EDITOR
#ifndef AK_OPTIMIZED
	const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
//	if (AkSettingsPerUser->XmlTranslatorTimeout > 0)
//	{
//		Monitor->SetupDefaultXMLErrorTranslator(AkSettingsPerUser->XmlTranslatorTimeout); TODO: see WG-60849
//	}

	if (AkSettingsPerUser->WaapiTranslatorTimeout > 0)
	{
#if AK_SUPPORT_WAAPI
		Monitor->SetupDefaultWAAPIErrorTranslator(AkSettingsPerUser->WaapiIPAddress, AkSettingsPerUser->WaapiPort, AkSettingsPerUser->WaapiTranslatorTimeout);
#endif //AK_SUPPORT_WAAPI
	}

	Monitor->AddTranslator(&m_UnrealErrorTranslator);

#endif //AK_OPTIMIZED
#endif //WITH_EDITOR
}

void FAkAudioDevice::AddDefaultListener(UAkComponent* in_pListener)
{
	bool bAlreadyInSet;
	m_defaultListeners.Add(in_pListener, &bAlreadyInSet);
	if (!bAlreadyInSet)
	{
		for (auto& Emitter : m_defaultEmitters)
			Emitter->OnDefaultListenerAdded(in_pListener);

		in_pListener->IsDefaultListener = true;
		UpdateDefaultActiveListeners();

		if (m_SpatialAudioListener == nullptr)
			SetSpatialAudioListener(in_pListener);
	}
}

void FAkAudioDevice::RemoveDefaultListener(UAkComponent* in_pListener)
{
	for (auto& Emitter : m_defaultEmitters)
	{
		Emitter->OnListenerUnregistered(in_pListener);
	}

	m_defaultListeners.Remove(in_pListener);
	in_pListener->IsDefaultListener = false;
	UpdateDefaultActiveListeners();

	// We are setting Aux Sends with the SpatialAudio API, and that requires a Spatial Audio listener.
	// When running dedicated server, Unreal creates a camera manager (default listener 1 gets set as spatial audio listener), then another one (default listener 2), and then destroys the first. This leaves us with a default listener, but no spatial audio listener. This fix targets that issue.
	if (m_SpatialAudioListener == in_pListener )
	{
		// Unregister the Spatial Audio Listener if its game object is unregistered 
		auto* SpatialAudio = FWwiseLowLevelSpatialAudio::Get();
		if (LIKELY(SpatialAudio))
		{
			SpatialAudio->UnregisterListener(m_SpatialAudioListener->GetAkGameObjectID());
		}
		m_SpatialAudioListener = nullptr;

		if (m_defaultListeners.Num() > 0)
		{
			for (auto listener : m_defaultListeners)
			{
				if (SetSpatialAudioListener(m_defaultListeners.Array()[0]))
				{
					break;
				}
			}
		}
	}
}

void FAkAudioDevice::OnActorSpawned(AActor* SpawnedActor)
{
	APlayerCameraManager* AsPlayerCameraManager = Cast<APlayerCameraManager>(SpawnedActor);
	if (AsPlayerCameraManager && AsPlayerCameraManager->GetWorld()->AllowAudioPlayback())
	{
		APlayerController* CameraOwner = Cast<APlayerController>(AsPlayerCameraManager->GetOwner());
		if (CameraOwner && CameraOwner->IsLocalPlayerController())
		{
			UAkComponent* pAkComponent = NewObject<UAkComponent>(SpawnedActor);
			if (pAkComponent != nullptr)
			{
				pAkComponent->RegisterComponentWithWorld(SpawnedActor->GetWorld());
				pAkComponent->AttachToComponent(SpawnedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform, FName());
				AddDefaultListener(pAkComponent);
			}
		}
	}
}

FString FAkAudioDevice::GetBasePath()
{
	return AkUnrealHelper::GetSoundBankDirectory();
}

/**
 * Allocates memory from permanent pool. This memory will NEVER be freed.
 *
 * @param	Size	Size of allocation.
 *
 * @return pointer to a chunk of memory with size Size
 */
void* FAkAudioDevice::AllocatePermanentMemory( int32 Size, bool& AllocatedInPool )
{
	return 0;
}

AKRESULT FAkAudioDevice::GetGameObjectID( AActor * in_pActor, AkGameObjectID& io_GameObject )
{
	if ( IsValid(in_pActor) )
	{
		UAkComponent * pComponent = GetAkComponent( in_pActor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset );
		if ( pComponent )
		{
			io_GameObject = pComponent->GetAkGameObjectID();
			return AK_Success;
		}
		else
			return AK_Fail;
	}

	// we do not modify io_GameObject, letting it to the specified default value.
	return AK_Success;
}

AKRESULT FAkAudioDevice::GetGameObjectID( AActor * in_pActor, AkGameObjectID& io_GameObject, bool in_bStopWhenOwnerDestroyed )
{
	if ( IsValid(in_pActor) )
	{
		UAkComponent * pComponent = GetAkComponent( in_pActor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset );
		if ( pComponent )
		{
			pComponent->StopWhenOwnerDestroyed = in_bStopWhenOwnerDestroyed;
			io_GameObject = pComponent->GetAkGameObjectID();
			return AK_Success;
		}
		else
			return AK_Fail;
	}

	// we do not modify io_GameObject, letting it to the specified default value.
	return AK_Success;
}

void FAkAudioDevice::Suspend(bool in_bRenderAnyway /* = false */)
{
	if (!m_isSuspended)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->Suspend(in_bRenderAnyway);
		m_isSuspended = true;
	}
}

void FAkAudioDevice::WakeupFromSuspend()
{
	if (m_isSuspended)
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->WakeupFromSuspend();
		m_isSuspended = false;
	}
}

void FAkAudioDevice::StartOutputCapture(const FString& Filename)
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StartOutputCapture(TCHAR_TO_AK(*Filename));
	}
}

void FAkAudioDevice::StopOutputCapture()
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StopOutputCapture();
	}
}

void FAkAudioDevice::StartProfilerCapture(const FString& Filename)
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StartProfilerCapture(TCHAR_TO_AK(*Filename));
	}
}

void FAkAudioDevice::AddOutputCaptureMarker(const FString& MarkerText)
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->AddOutputCaptureMarker(TCHAR_TO_ANSI(*MarkerText));
	}
}

void FAkAudioDevice::StopProfilerCapture()
{
	if ( m_bSoundEngineInitialized )
	{
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StopProfilerCapture();
	}
}

AKRESULT FAkAudioDevice::RegisterPluginDLL(const FString& in_DllName, const FString& in_DllPath)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return AK_NotInitialized;

	AkOSChar* szPath = nullptr;

	if (!in_DllPath.IsEmpty())
	{
		auto Length = in_DllPath.Len() + 1;
		szPath = new AkOSChar[Length];
		AKPLATFORM::SafeStrCpy(szPath, TCHAR_TO_AK(*in_DllPath), Length);
	}

	AKRESULT eResult = SoundEngine->RegisterPluginDLL(TCHAR_TO_AK(*in_DllName), szPath);
	delete[] szPath;
	return eResult;
}
// end

FAkAudioDevice::SetCurrentAudioCultureAsyncTask::SetCurrentAudioCultureAsyncTask(FWwiseLanguageCookedData NewLanguage, FSetCurrentAudioCultureAction* LatentAction)
	: Language(NewLanguage)
	, SetAudioCultureLatentAction(LatentAction)
{
	CompletionActionType = CompletionType::LatentAction;
	LatentActionValidityToken = MakeShared<FPendingLatentActionValidityToken, ESPMode::ThreadSafe>();
	SetAudioCultureLatentAction->ValidityToken = LatentActionValidityToken;
}

FAkAudioDevice::SetCurrentAudioCultureAsyncTask::SetCurrentAudioCultureAsyncTask(FWwiseLanguageCookedData NewLanguage, const FOnSetCurrentAudioCultureCompleted& CompletedCallback)
	: Language(NewLanguage)
	, SetAudioCultureCompletedCallback(CompletedCallback)
{
	CompletionActionType = CompletionType::Callback;
}

bool FAkAudioDevice::SetCurrentAudioCultureAsyncTask::Start()
{
	UE_LOG(LogAkAudio, Verbose, TEXT("Switching Wwise language to '%s'"), *Language.GetLanguageName());

	auto* StreamMgr = FWwiseLowLevelStreamMgr::Get();
	if (UNLIKELY(!StreamMgr))
	{
		return false;
	}
	StreamMgr->SetCurrentLanguage(TCHAR_TO_AK(*Language.GetLanguageName()));

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this]()
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			UE_LOG(LogAkAudio, Error, TEXT("SetCurrentAudioCultureAsync: Could not get resource loader, cannot change language."));
			Succeeded = false;
			IsDone = true;
			return;
		}

		ResourceLoader->SetLanguage(Language, EWwiseReloadLanguage::Immediate);
		IsDone = true;
		Succeeded = true;
	});

	return true;
}

void FAkAudioDevice::SetCurrentAudioCultureAsyncTask::Update()
{
	if (IsDone)
	{
		switch (CompletionActionType) 
		{
			case CompletionType::Callback:
				SetAudioCultureCompletedCallback.ExecuteIfBound(Succeeded);
				break;
			case CompletionType::LatentAction:
				if (LatentActionValidityToken->bValid && SetAudioCultureLatentAction)
				{
					SetAudioCultureLatentAction->ActionDone = true;
				}
				break;
		}
	}
}

bool FAkAudioDevice::IsPlayingIDActive(uint32 EventID, uint32 PlayingID)
{
	FScopeLock Lock(&EventToPlayingIDMapCriticalSection);
	auto* PlayingIDArray = EventToPlayingIDMap.Find(EventID);
	if (PlayingIDArray && PlayingIDArray->Contains(PlayingID))
	{
		return true;
	}

	return false;
}

bool FAkAudioDevice::IsEventIDActive(uint32 EventID)
{
	FScopeLock Lock(&EventToPlayingIDMapCriticalSection);
	return EventToPlayingIDMap.Contains(EventID);
}

void FAkAudioDevice::RemovePlayingID(uint32 EventID, uint32 PlayingID)
{
	FScopeLock Lock(&EventToPlayingIDMapCriticalSection);
	auto* PlayingIDArray = EventToPlayingIDMap.Find(EventID);
	if (PlayingIDArray)
	{
		PlayingIDArray->Remove(PlayingID);
		if (PlayingIDArray->Num() == 0)
		{
			EventToPlayingIDMap.Remove(EventID);
		}
	}
}

void FAkAudioDevice::StopEventID(uint32 EventID)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine)) return;

	FScopeLock Lock(&EventToPlayingIDMapCriticalSection);

	auto* PlayingIDs = EventToPlayingIDMap.Find(EventID);
	if (PlayingIDs)
	{
		for (auto pID : *PlayingIDs)
		{
			StopPlayingID(pID);
		}
		SoundEngine->RenderAudio();
	}
}

FOnSwitchValueLoaded& FAkAudioDevice::GetOnSwitchValueLoaded(uint32 SwitchID)
{
	return OnSwitchValueLoadedMap.FindOrAdd(SwitchID);
}

void FAkAudioDevice::BroadcastOnSwitchValueLoaded(UAkGroupValue* GroupValue)
{
	FOnSwitchValueLoaded* EventToBroadcast = OnSwitchValueLoadedMap.Find(GroupValue->GroupValueCookedData.Id);
	if (EventToBroadcast)
	{
		EventToBroadcast->Broadcast(GroupValue);
	}
}
