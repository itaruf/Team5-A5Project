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

#include "Wwise/WwiseResourceLoaderImpl.h"

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"

#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"

#include <inttypes.h>

#include "Wwise/WwiseGlobalCallbacks.h"


FWwiseResourceLoaderImpl::FWwiseResourceLoaderImpl()
{
}

FName FWwiseResourceLoaderImpl::GetUnrealExternalSourcePath() const
{
#if WITH_EDITORONLY_DATA
	return FName(GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString() / CurrentPlatform.Platform->ExternalSourceRootPath.ToString());
#else
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealExternalSourcePath: Failed to retrieve External Source Manager, returning empty string."));
		return {};
	}
	return FName(FPaths::ProjectContentDir() / ExternalSourceManager->GetStagingDirectory());
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealPath() const
{
#if WITH_EDITOR
	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
#elif WITH_EDITORONLY_DATA
	UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealPath should not be used in WITH_EDITORONLY_DATA (Getting path for %s)"), *InPath);
	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks;
#else
	return StagePath;
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealPath(const FString& InPath) const
{
#if WITH_EDITOR
	return GetUnrealGeneratedSoundBanksPath(InPath);
#elif WITH_EDITORONLY_DATA
	UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealPath should not be used in WITH_EDITORONLY_DATA (Getting path for %s)"), *InPath);
	return GetUnrealGeneratedSoundBanksPath(InPath);
#else
	return GetUnrealStagePath(InPath);
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealStagePath(const FString& InPath) const
{
	if (UNLIKELY(StagePath.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("StagePath not set up (GetUnrealStagePath for %s)"), *InPath);
	}
	return StagePath / InPath;
}

#if WITH_EDITORONLY_DATA
FString FWwiseResourceLoaderImpl::GetUnrealGeneratedSoundBanksPath(const FString& InPath) const
{
	if (UNLIKELY(GeneratedSoundBanksPath.Path.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("GeneratedSoundBanksPath not set up (GetUnrealGeneratedSoundBanksPath for %s)"), *InPath);
	}

	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString() / InPath;
}
#endif


EWwiseResourceLoaderState FWwiseResourceLoaderImpl::GetResourceLoaderState()
{
	return WwiseResourceLoaderState;
}

void FWwiseResourceLoaderImpl::SetResourceLoaderState(EWwiseResourceLoaderState State)
{
	WwiseResourceLoaderState = State;
}

bool FWwiseResourceLoaderImpl::IsEnabled()
{
	return WwiseResourceLoaderState == EWwiseResourceLoaderState::Enabled;
}

void FWwiseResourceLoaderImpl::Disable()
{
	SetResourceLoaderState(EWwiseResourceLoaderState::AlwaysDisabled);
}

void FWwiseResourceLoaderImpl::Enable()
{
	SetResourceLoaderState(EWwiseResourceLoaderState::Enabled);
}

void FWwiseResourceLoaderImpl::SetLanguage(const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);

	const auto OldLanguage = CurrentLanguage;
	const auto NewLanguage = InLanguage;

	if (OldLanguage == NewLanguage)
	{
		return;
	}

	UE_CLOG(!OldLanguage.GetLanguageName().IsValid(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] To %s"), *NewLanguage.GetLanguageName().ToString());
	UE_CLOG(OldLanguage.GetLanguageName().IsValid(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] from %s to %s"), *OldLanguage.GetLanguageName().ToString(), *NewLanguage.GetLanguageName().ToString());


	if (InReloadLanguage == EWwiseReloadLanguage::Safe)
	{
		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Stopping all sounds"));
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: SoundEngine not available to stop all sounds"));
		}
		else
		{
			SoundEngine->StopAll();
			auto* WwiseGlobalCallbacks = FWwiseGlobalCallbacks::Get();
			if(WwiseGlobalCallbacks)
			{
				// Wait two audio processing passes to make sure our StopAll was processed.
				WwiseGlobalCallbacks->WaitForEnd();
				WwiseGlobalCallbacks->WaitForEnd();
			}
		}
	}

	CurrentLanguage = NewLanguage;

	if (InReloadLanguage != EWwiseReloadLanguage::Manual)
	{
		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Immediately switching languages. Unloading old language %s."),
			*OldLanguage.GetLanguageName().ToString());

		ListExecutionQueue.AsyncWait([this, &OldLanguage, &NewLanguage]() mutable
		{
			for (auto& LoadedSoundBank : LoadedSoundBankList)
			{
				if (LoadedSoundBank.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
				if (LIKELY(SoundBank))
				{
					UnloadSoundBankResources(LoadedSoundBank.LoadedData, *SoundBank);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedAuxBus : LoadedAuxBusList)
			{
				if (LoadedAuxBus.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
				if (LIKELY(AuxBus))
				{
					UnloadAuxBusResources(LoadedAuxBus.LoadedData, *AuxBus);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedShareSet : LoadedShareSetList)
			{
				if (LoadedShareSet.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping ShareSet %s with language %s"),
						*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
				if (LIKELY(ShareSet))
				{
					UnloadShareSetResources(LoadedShareSet.LoadedData, *ShareSet);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find ShareSet %s with language %s"),
						*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet.LanguageRef.GetLanguageName().ToString());
				}
			}
			for (auto& LoadedEvent : LoadedEventList)
			{
				if (LoadedEvent.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
				if (LIKELY(Event))
				{
					UnloadEventResources(LoadedEvent.LoadedData, *Event);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent.LanguageRef.GetLanguageName().ToString());
				}
			}


			UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Loading new language %s."),
				*NewLanguage.GetLanguageName().ToString());

			for (auto& LoadedSoundBank : LoadedSoundBankList)
			{
				if (LoadedSoundBank.LanguageRef != OldLanguage)
				{
					continue;
				}

				LoadedSoundBank.LanguageRef = NewLanguage;
				auto* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
				if (LIKELY(SoundBank))
				{
					LoadSoundBankResources(LoadedSoundBank.LoadedData, *SoundBank);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedAuxBus : LoadedAuxBusList)
			{
				if (LoadedAuxBus.LanguageRef != OldLanguage)
				{
					continue;
				}

				LoadedAuxBus.LanguageRef = NewLanguage;
				auto* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
				if (LIKELY(AuxBus))
				{
					LoadAuxBusResources(LoadedAuxBus.LoadedData, *AuxBus);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedShareSet : LoadedShareSetList)
			{
				if (LoadedShareSet.LanguageRef != OldLanguage)
				{
					continue;
				}

				LoadedShareSet.LanguageRef = NewLanguage;
				auto* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
				if (LIKELY(ShareSet))
				{
					LoadShareSetResources(LoadedShareSet.LoadedData, *ShareSet);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find ShareSet %s with language %s"),
						*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedEvent : LoadedEventList)
			{
				if (LoadedEvent.LanguageRef != OldLanguage)
				{
					continue;
				}

				LoadedEvent.LanguageRef = NewLanguage;
				auto* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
				if (LIKELY(Event))
				{
					LoadEventResources(LoadedEvent.LoadedData, *Event);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent.LanguageRef.GetLanguageName().ToString());
				}
			}
		});
	}

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Done"));
}

void FWwiseResourceLoaderImpl::SetPlatform(const FWwiseSharedPlatformId& InPlatform)
{
	UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetPlatform: Updating platform from %s (%s) to %s (%s)."),
		*CurrentPlatform.GetPlatformName().ToString(), *CurrentPlatform.GetPlatformGuid().ToString(),
		*InPlatform.GetPlatformName().ToString(), *InPlatform.GetPlatformGuid().ToString());

	CurrentPlatform = InPlatform;
}


FWwiseLoadedAuxBus FWwiseResourceLoaderImpl::CreateAuxBusNode(
	const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InAuxBusCookedData.AuxBusLanguageMap, InLanguageOverride, InAuxBusCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateAuxBusNode: Could not find language for Aux Bus %s"), *InAuxBusCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedAuxBusListNode(FWwiseLoadedAuxBusInfo(InAuxBusCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadAuxBusAsync(FWwiseLoadedAuxBusPromise&& Promise, FWwiseLoadedAuxBus&& InAuxBusListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedAuxBus = InAuxBusListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
	const FWwiseAuxBusCookedData* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusAsync: Could not find AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
		delete InAuxBusListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	AsyncTask(TaskThread, [this, &LoadedAuxBus, AuxBus, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadAuxBusResources(LoadedAuxBus.LoadedData, *AuxBus)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusAsync: Could not load AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
			delete InAuxBusListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachAuxBusNode(InAuxBusListNode);

			Timing.Stop();
			Promise.EmplaceValue(InAuxBusListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadAuxBusAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBus&& InAuxBusListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedAuxBus = InAuxBusListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);

	const FWwiseAuxBusCookedData* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadAuxBusAsync: Could not find AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	ListExecutionQueue.Async([this, &LoadedAuxBus, AuxBus, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachAuxBusNode(InAuxBusListNode);

		AsyncTask(TaskThread, [this, &LoadedAuxBus, AuxBus, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadAuxBusResources(LoadedAuxBus.LoadedData, *AuxBus);
			delete InAuxBusListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedEvent FWwiseResourceLoaderImpl::CreateEventNode(
	const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InEventCookedData.EventLanguageMap, InLanguageOverride, InEventCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateEventNode: Could not find language for Event %s"), *InEventCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedEventListNode(FWwiseLoadedEventInfo(InEventCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadEventAsync(FWwiseLoadedEventPromise&& Promise, FWwiseLoadedEvent&& InEventListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedEvent = InEventListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
	const FWwiseEventCookedData* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventAsync: Could not find Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
		delete InEventListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	AsyncTask(TaskThread, [this, &LoadedEvent, Event, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadEventResources(LoadedEvent.LoadedData, *Event)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventAsync: Could not load Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
			delete InEventListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachEventNode(InEventListNode);

			Timing.Stop();
			Promise.EmplaceValue(InEventListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadEventAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEvent&& InEventListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedEvent = InEventListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);

	const FWwiseEventCookedData* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadEventAsync: Could not find Event %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	ListExecutionQueue.Async([this, &LoadedEvent, Event, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachEventNode(InEventListNode);

		AsyncTask(TaskThread, [this, &LoadedEvent, Event, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadEventResources(LoadedEvent.LoadedData, *Event);
			delete InEventListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedExternalSource FWwiseResourceLoaderImpl::CreateExternalSourceNode(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	return new FWwiseLoadedExternalSourceListNode(FWwiseLoadedExternalSourceInfo(InExternalSourceCookedData));
}

void FWwiseResourceLoaderImpl::LoadExternalSourceAsync(FWwiseLoadedExternalSourcePromise&& Promise, FWwiseLoadedExternalSource&& InExternalSourceListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedExternalSource = InExternalSourceListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading ExternalSource %s (%" PRIu32 ")"),
		*LoadedExternalSource.ExternalSourceCookedData.DebugName.ToString(), LoadedExternalSource.ExternalSourceCookedData.Cookie);
	const FWwiseExternalSourceCookedData* ExternalSource = &LoadedExternalSource.ExternalSourceCookedData;

	AsyncTask(TaskThread, [this, &LoadedExternalSource, ExternalSource, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadExternalSourceResources(LoadedExternalSource.LoadedData, *ExternalSource)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceAsync: Could not load ExternalSource %s (%" PRIu32 ")"),
			*LoadedExternalSource.ExternalSourceCookedData.DebugName.ToString(), LoadedExternalSource.ExternalSourceCookedData.Cookie);
			delete InExternalSourceListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachExternalSourceNode(InExternalSourceListNode);

			Timing.Stop();
			Promise.EmplaceValue(InExternalSourceListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSource&& InExternalSourceListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedExternalSource = InExternalSourceListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading ExternalSource %s (%" PRIu32 ")"),
		*LoadedExternalSource.ExternalSourceCookedData.DebugName.ToString(), LoadedExternalSource.ExternalSourceCookedData.Cookie);

	const FWwiseExternalSourceCookedData* ExternalSource = &LoadedExternalSource.ExternalSourceCookedData;

	ListExecutionQueue.Async([this, &LoadedExternalSource, ExternalSource, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachExternalSourceNode(InExternalSourceListNode);

		AsyncTask(TaskThread, [this, &LoadedExternalSource, ExternalSource, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadExternalSourceResources(LoadedExternalSource.LoadedData, *ExternalSource);
			delete InExternalSourceListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedGroupValue FWwiseResourceLoaderImpl::CreateGroupValueNode(
	const FWwiseGroupValueCookedData& InGroupValueCookedData)
{
	return new FWwiseLoadedGroupValueListNode(FWwiseLoadedGroupValueInfo(InGroupValueCookedData));
}

void FWwiseResourceLoaderImpl::LoadGroupValueAsync(FWwiseLoadedGroupValuePromise&& Promise, FWwiseLoadedGroupValue&& InGroupValueListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedGroupValue = InGroupValueListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading GroupValue %s (%s %" PRIu32 ":%" PRIu32 ")"),
		*LoadedGroupValue.GroupValueCookedData.DebugName.ToString(), *LoadedGroupValue.GroupValueCookedData.GetTypeName(), LoadedGroupValue.GroupValueCookedData.GroupId, LoadedGroupValue.GroupValueCookedData.Id);
	const FWwiseGroupValueCookedData* GroupValue = &LoadedGroupValue.GroupValueCookedData;

	SwitchContainerExecutionQueue.Async([this, &LoadedGroupValue, GroupValue, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadGroupValueResources(LoadedGroupValue.LoadedData, *GroupValue)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadGroupValueAsync: Could not load GroupValue %s (%s %" PRIu32 ":%" PRIu32 ")"),
				*LoadedGroupValue.GroupValueCookedData.DebugName.ToString(), *LoadedGroupValue.GroupValueCookedData.GetTypeName(), LoadedGroupValue.GroupValueCookedData.GroupId, LoadedGroupValue.GroupValueCookedData.Id);
			delete InGroupValueListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachGroupValueNode(InGroupValueListNode);

			Timing.Stop();
			Promise.EmplaceValue(InGroupValueListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadGroupValueAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValue&& InGroupValueListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedGroupValue = InGroupValueListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading GroupValue %s (%s %" PRIu32 ":%" PRIu32 ")"),
		*LoadedGroupValue.GroupValueCookedData.DebugName.ToString(), *LoadedGroupValue.GroupValueCookedData.GetTypeName(), LoadedGroupValue.GroupValueCookedData.GroupId, LoadedGroupValue.GroupValueCookedData.Id);

	const FWwiseGroupValueCookedData* GroupValue = &LoadedGroupValue.GroupValueCookedData;

	ListExecutionQueue.Async([this, &LoadedGroupValue, GroupValue, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachGroupValueNode(InGroupValueListNode);

		SwitchContainerExecutionQueue.Async([this, &LoadedGroupValue, GroupValue, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadGroupValueResources(LoadedGroupValue.LoadedData, *GroupValue);
			delete InGroupValueListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedInitBank FWwiseResourceLoaderImpl::CreateInitBankNode(
	const FWwiseInitBankCookedData& InInitBankCookedData)
{
	return new FWwiseLoadedInitBankListNode(FWwiseLoadedInitBankInfo(InInitBankCookedData));
}

void FWwiseResourceLoaderImpl::LoadInitBankAsync(FWwiseLoadedInitBankPromise&& Promise, FWwiseLoadedInitBank&& InInitBankListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedInitBank = InInitBankListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading InitBank %s (%" PRIu32 ")"),
		*LoadedInitBank.InitBankCookedData.DebugName.ToString(), LoadedInitBank.InitBankCookedData.SoundBankId);
	const FWwiseInitBankCookedData* InitBank = &LoadedInitBank.InitBankCookedData;

	AsyncTask(TaskThread, [this, &LoadedInitBank, InitBank, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadInitBankResources(LoadedInitBank.LoadedData, *InitBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankAsync: Could not load InitBank %s (%" PRIu32 ")"),
			*LoadedInitBank.InitBankCookedData.DebugName.ToString(), LoadedInitBank.InitBankCookedData.SoundBankId);
			delete InInitBankListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachInitBankNode(InInitBankListNode);

			Timing.Stop();
			Promise.EmplaceValue(InInitBankListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadInitBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBank&& InInitBankListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedInitBank = InInitBankListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading InitBank %s (%" PRIu32 ")"),
		*LoadedInitBank.InitBankCookedData.DebugName.ToString(), LoadedInitBank.InitBankCookedData.SoundBankId);

	const FWwiseInitBankCookedData* InitBank = &LoadedInitBank.InitBankCookedData;

	ListExecutionQueue.Async([this, &LoadedInitBank, InitBank, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachInitBankNode(InInitBankListNode);

		AsyncTask(TaskThread, [this, &LoadedInitBank, InitBank, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadInitBankResources(LoadedInitBank.LoadedData, *InitBank);
			delete InInitBankListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedMedia FWwiseResourceLoaderImpl::CreateMediaNode(const FWwiseMediaCookedData& InMediaCookedData)
{
	return new FWwiseLoadedMediaListNode(FWwiseLoadedMediaInfo(InMediaCookedData));
}

void FWwiseResourceLoaderImpl::LoadMediaAsync(FWwiseLoadedMediaPromise&& Promise, FWwiseLoadedMedia&& InMediaListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedMedia = InMediaListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Media %s (%" PRIu32 ")"),
		*LoadedMedia.MediaCookedData.DebugName.ToString(), LoadedMedia.MediaCookedData.MediaId);
	const FWwiseMediaCookedData* Media = &LoadedMedia.MediaCookedData;

	AsyncTask(TaskThread, [this, &LoadedMedia, Media, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadMediaResources(LoadedMedia.LoadedData, *Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaAsync: Could not load Media %s (%" PRIu32 ")"),
			*LoadedMedia.MediaCookedData.DebugName.ToString(), LoadedMedia.MediaCookedData.MediaId);
			delete InMediaListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachMediaNode(InMediaListNode);

			Timing.Stop();
			Promise.EmplaceValue(InMediaListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadMediaAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMedia&& InMediaListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedMedia = InMediaListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Media %s (%" PRIu32 ")"),
		*LoadedMedia.MediaCookedData.DebugName.ToString(), LoadedMedia.MediaCookedData.MediaId);

	const FWwiseMediaCookedData* Media = &LoadedMedia.MediaCookedData;

	ListExecutionQueue.Async([this, &LoadedMedia, Media, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachMediaNode(InMediaListNode);

		AsyncTask(TaskThread, [this, &LoadedMedia, Media, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadMediaResources(LoadedMedia.LoadedData, *Media);
			delete InMediaListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


FWwiseLoadedShareSet FWwiseResourceLoaderImpl::CreateShareSetNode(
	const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InShareSetCookedData.ShareSetLanguageMap, InLanguageOverride, InShareSetCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateShareSetNode: Could not find language for ShareSet %s"), *InShareSetCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedShareSetListNode(FWwiseLoadedShareSetInfo(InShareSetCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadShareSetAsync(FWwiseLoadedShareSetPromise&& Promise, FWwiseLoadedShareSet&& InShareSetListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedShareSet = InShareSetListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
	const FWwiseShareSetCookedData* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
	if (UNLIKELY(!ShareSet))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetAsync: Could not find ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
		delete InShareSetListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	AsyncTask(TaskThread, [this, &LoadedShareSet, ShareSet, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadShareSetResources(LoadedShareSet.LoadedData, *ShareSet)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetAsync: Could not load ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
			delete InShareSetListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachShareSetNode(InShareSetListNode);

			Timing.Stop();
			Promise.EmplaceValue(InShareSetListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadShareSetAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSet&& InShareSetListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedShareSet = InShareSetListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);

	const FWwiseShareSetCookedData* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
	if (UNLIKELY(!ShareSet))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadShareSetAsync: Could not find ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	ListExecutionQueue.Async([this, &LoadedShareSet, ShareSet, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachShareSetNode(InShareSetListNode);

		AsyncTask(TaskThread, [this, &LoadedShareSet, ShareSet, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadShareSetResources(LoadedShareSet.LoadedData, *ShareSet);
			delete InShareSetListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}

FWwiseLoadedSoundBank FWwiseResourceLoaderImpl::CreateSoundBankNode(
	const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InSoundBankCookedData.SoundBankLanguageMap, InLanguageOverride, InSoundBankCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateSoundBankNode: Could not find language for SoundBank %s"), *InSoundBankCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedSoundBankListNode(FWwiseLoadedSoundBankInfo(InSoundBankCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadSoundBankAsync(FWwiseLoadedSoundBankPromise&& Promise, FWwiseLoadedSoundBank&& InSoundBankListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedSoundBank = InSoundBankListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
	const FWwiseSoundBankCookedData* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankAsync: Could not find SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
		delete InSoundBankListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	AsyncTask(TaskThread, [this, &LoadedSoundBank, SoundBank, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		if (UNLIKELY(!LoadSoundBankResources(LoadedSoundBank.LoadedData, *SoundBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankAsync: Could not load SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
			delete InSoundBankListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		ListExecutionQueue.Async([this, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			AttachSoundBankNode(InSoundBankListNode);

			Timing.Stop();
			Promise.EmplaceValue(InSoundBankListNode);
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadSoundBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBank&& InSoundBankListNode)
{
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedSoundBank = InSoundBankListNode->GetValue();

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
		*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);

	const FWwiseSoundBankCookedData* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadSoundBankAsync: Could not find SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	ListExecutionQueue.Async([this, &LoadedSoundBank, SoundBank, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
	{
		DetachSoundBankNode(InSoundBankListNode);

		AsyncTask(TaskThread, [this, &LoadedSoundBank, SoundBank, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)]() mutable
		{
			UnloadSoundBankResources(LoadedSoundBank.LoadedData, *SoundBank);
			delete InSoundBankListNode;

			Timing.Stop();
			Promise.EmplaceValue();
		});
	});
}


bool FWwiseResourceLoaderImpl::LoadAuxBusResources(FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading AuxBus %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusResources: AuxBus %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
		return false;
	}

	FCompletionFutureArray FutureArray;

	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("AuxBus"), InCookedData.DebugName, InCookedData.AuxBusId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("AuxBus"), InCookedData.DebugName, InCookedData.AuxBusId);
	WaitForFutures(FutureArray);

	if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusResources: Could not load %d prerequisites for AuxBus %s (%" PRIu32 "). Unloading and failing."),
			InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
		UnloadAuxBusResources(LoadedData, InCookedData);
		return false;
	}
	return true;
}


bool FWwiseResourceLoaderImpl::LoadEventResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Event %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Event %s (%" PRIu32 ") is already loaded."),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
		return false;
	}

	FCompletionPromise SwitchContainerPromise;
	auto SwitchContainerFuture = SwitchContainerPromise.GetFuture();
	if (InCookedData.RequiredGroupValueSet.Num() > 0 || InCookedData.SwitchContainerLeaves.Num() > 0)
	{
		AsyncTask(TaskThread, [this, &InCookedData, &LoadedData, SwitchContainerPromise = MoveTemp(SwitchContainerPromise)]() mutable
		{
			LoadEventSwitchContainerResources(LoadedData, InCookedData);
			SwitchContainerPromise.EmplaceValue();
		});
	}
	else
	{
		SwitchContainerPromise.EmplaceValue();
	}

	FCompletionFutureArray FutureArray;

	AddLoadExternalSourceFutures(FutureArray, LoadedExternalSources, InCookedData.ExternalSources, TEXT("Event"), InCookedData.DebugName, InCookedData.EventId);
	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("Event"), InCookedData.DebugName, InCookedData.EventId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("Event"), InCookedData.DebugName, InCookedData.EventId);
	WaitForFutures(FutureArray);
	SwitchContainerFuture.Wait();

	if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load %d prerequisites for Event %s (%" PRIu32 "). Unloading and failing."),
			InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
		UnloadEventResources(LoadedData, InCookedData);
		return false;
	}
	return true;
}

void FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	// Load required GroupValues
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)InCookedData.RequiredGroupValueSet.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	FWwiseLoadedGroupValueList& LoadedRequiredGroupValues = LoadedData.LoadedRequiredGroupValues;
	bool& bLoadedSwitchContainerLeaves = LoadedData.bLoadedSwitchContainerLeaves;
	TArray<TFuture<FWwiseLoadedGroupValue>> GroupValueFutureArray;
	FCompletionFutureArray LeavesFutureArray;

	for (const auto& GroupValue : InCookedData.RequiredGroupValueSet)
	{
		TPromise<FWwiseLoadedGroupValue> Promise;
		GroupValueFutureArray.Add(Promise.GetFuture());

		SwitchContainerExecutionQueue.Async([this, &InCookedData, &GroupValue, Promise = MoveTemp(Promise)]() mutable
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading GroupValue %s for Event %s (%" PRIu32 ")"),
				*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
			FWwiseLoadedGroupValueInfo::FLoadedData GroupValueLoadedData;
			if (UNLIKELY(!LoadGroupValueResources(GroupValueLoadedData, GroupValue) || !GroupValueLoadedData.IsLoaded()))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not load required GroupValue %s for Event %s (%" PRIu32 ")"),
					*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
				Promise.EmplaceValue(nullptr);
			}
			else
			{
				FWwiseLoadedGroupValueInfo LoadedGroupValue(GroupValue);
				auto* LoadedNode = new FWwiseLoadedGroupValueListNode(LoadedGroupValue);
				Promise.EmplaceValue(LoadedNode);
			}
		});
	}

	// Load Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		check(SwitchContainerLeaf.GroupValueSet.Num() > 0);
		FWwiseSwitchContainerLeafGroupValueUsageCount* UsageCount = new FWwiseSwitchContainerLeafGroupValueUsageCount(SwitchContainerLeaf);
		for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
		{
			FCompletionPromise Promise;
			LeavesFutureArray.Add(Promise.GetFuture());

			SwitchContainerExecutionQueue.Async([this, &InCookedData, &GroupValue, UsageCount, Promise = MoveTemp(Promise)]() mutable
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Adding requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
					*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

				auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue));
				auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue), nullptr);
				FWwiseSwitchContainerLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
				bool bIsAlreadyCreated = false;
				auto UsageCountId = Info.Leaves.Add(UsageCount, &bIsAlreadyCreated);
				check(!bIsAlreadyCreated);
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadCount: %d"), Info.LoadCount);
				if (Info.ShouldBeLoaded())
				{
					bIsAlreadyCreated = false;
					UsageCount->LoadedGroupValues.Add(GroupValue, &bIsAlreadyCreated);
					check(!bIsAlreadyCreated);
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), (int)UsageCount->LoadedGroupValues.Num());
					if (UsageCount->HaveAllKeys())
					{
						check(!UsageCount->LoadedData.IsLoaded());
						LoadSwitchContainerLeafResources(UsageCount->LoadedData, UsageCount->Key);
					}
				}
				Promise.EmplaceValue();
			});
		}
	}

	// Wait for tasks to be done, and set up the LoadedData values
	for (auto& Future : GroupValueFutureArray)
	{
		if (auto* LoadedNode = Future.Get())
		{
			LoadedRequiredGroupValues.AddTail(LoadedNode);
		}
	}
	for (auto& Future : LeavesFutureArray)
	{
		Future.Wait();
	}
	bLoadedSwitchContainerLeaves = LeavesFutureArray.Num() > 0;
}

bool FWwiseResourceLoaderImpl::LoadExternalSourceResources(FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading External Source %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceResources: ExternalSource %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);
		return false;
	}

	TPromise<bool> Promise;
	auto Future = Promise.GetFuture();
	LoadExternalSourceFile(InCookedData, [Promise = MoveTemp(Promise)](bool bInResult) mutable
	{
		Promise.EmplaceValue(bInResult);
	});
	LoadedData.bLoaded = Future.Get();

	if (UNLIKELY(!LoadedData.bLoaded))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceResources: Could not load ExternalSource %s (%" PRIu32 ")"),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);
		return false;
	}
	return true;
}

bool FWwiseResourceLoaderImpl::LoadGroupValueResources(FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);

	auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData));
	auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData), nullptr);
	FWwiseSwitchContainerLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
	const bool bWasLoaded = Info.ShouldBeLoaded();
	++Info.LoadCount;

	if (!bWasLoaded && Info.ShouldBeLoaded())
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("First GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") load. Loading %d leaves."),
			*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.Leaves.Num());

		for (auto* UsageCount : Info.Leaves)
		{
			bool bIsAlreadyCreated = false;
			UsageCount->LoadedGroupValues.Add(InCookedData, &bIsAlreadyCreated);
			check(!bIsAlreadyCreated);
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), UsageCount->LoadedGroupValues.Num());
			if (UsageCount->HaveAllKeys())
			{
				LoadSwitchContainerLeafResources(UsageCount->LoadedData, UsageCount->Key);
			}
		}
	}
	else
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") already loaded (Count: %d times)."),
			*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.LoadCount);
	}

	return true;
}

bool FWwiseResourceLoaderImpl::LoadInitBankResources(FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading InitBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);

	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: InitBank %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return false;
	}

	FCompletionFutureArray FutureArray;
	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("InitBank"), InCookedData.DebugName, InCookedData.SoundBankId);
	WaitForFutures(FutureArray);

	TPromise<bool> Promise;
	auto Future = Promise.GetFuture();
	LoadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise)](bool bInResult) mutable
	{
		Promise.EmplaceValue(bInResult);
	});
	LoadedData.bLoaded = Future.Get();

	if (UNLIKELY(!LoadedData.bLoaded))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: Could not load InitBank %s (%" PRIu32 ")"),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return false;
	}
	return true;
}

bool FWwiseResourceLoaderImpl::LoadMediaResources(FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Media %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaResources: Media %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);
		return false;
	}

	TPromise<bool> Promise;
	auto Future = Promise.GetFuture();
	LoadMediaFile(InCookedData, [Promise = MoveTemp(Promise)](bool bInResult) mutable
	{
		Promise.EmplaceValue(bInResult);
	});
	LoadedData.bLoaded = Future.Get();

	if (UNLIKELY(!LoadedData.bLoaded))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaResources: Could not load Media %s (%" PRIu32 ")"),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);
		return false;
	}
	return true;
}

bool FWwiseResourceLoaderImpl::LoadShareSetResources(FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading ShareSet %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetResources: ShareSet %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
		return false;
	}

	FCompletionFutureArray FutureArray;

	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("ShareSet"), InCookedData.DebugName, InCookedData.ShareSetId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("ShareSet"), InCookedData.DebugName, InCookedData.ShareSetId);
	WaitForFutures(FutureArray);

	if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetResources: Could not load %d SoundBanks for ShareSet %s (%" PRIu32 "). Unloading and failing."),
			InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
		UnloadShareSetResources(LoadedData, InCookedData);
		return false;
	}
	return true;
}

bool FWwiseResourceLoaderImpl::LoadSoundBankResources(FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankResources: SoundBank %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return false;
	}

	TPromise<bool> Promise;
	auto Future = Promise.GetFuture();
	LoadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise)](bool bInResult) mutable
	{
		Promise.EmplaceValue(bInResult);
	});
	LoadedData.bLoaded = Future.Get();

	if (UNLIKELY(!LoadedData.bLoaded))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankResources: Could not load SoundBank %s (%" PRIu32 ")"),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return false;
	}
	return true;
}

void FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources(FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData& LoadedData, const FWwiseSwitchContainerLeafCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Switch Container Leaf"));

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSwitchContainerLeafResources: Leaf is already loaded."));
		return;
	}

	FCompletionFutureArray FutureArray;

	AddLoadExternalSourceFutures(FutureArray, LoadedExternalSources, InCookedData.ExternalSources, TEXT("Leaf"), {}, 0);
	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("Leaf"), {}, 0);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("Leaf"), {}, 0);
	WaitForFutures(FutureArray);

	INC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);
}


void FWwiseResourceLoaderImpl::UnloadAuxBusResources(FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading AuxBus %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	FCompletionFutureArray FutureArray;

	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks);
	WaitForFutures(FutureArray);
	AddUnloadMediaFutures(FutureArray, LoadedMedia);
	WaitForFutures(FutureArray);
}

void FWwiseResourceLoaderImpl::UnloadEventResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Event %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	FCompletionFutureArray FutureArray;

	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks);
	WaitForFutures(FutureArray);

	if (LoadedData.bLoadedSwitchContainerLeaves || LoadedData.LoadedRequiredGroupValues.Num() > 0)
	{
		FCompletionPromise Promise;
		FutureArray.Add(Promise.GetFuture());
		AsyncTask(TaskThread, [this, &LoadedData, &InCookedData, Promise = MoveTemp(Promise)]() mutable
		{
			UnloadEventSwitchContainerResources(LoadedData, InCookedData);
			Promise.EmplaceValue();
		});
	}
	AddUnloadExternalSourceFutures(FutureArray, LoadedExternalSources);
	AddUnloadMediaFutures(FutureArray, LoadedMedia);
	WaitForFutures(FutureArray);
}

void FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	// Unload required GroupValues
	FWwiseLoadedGroupValueList& LoadedRequiredGroupValues = LoadedData.LoadedRequiredGroupValues;
	bool& bLoadedSwitchContainerLeaves = LoadedData.bLoadedSwitchContainerLeaves;

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)LoadedRequiredGroupValues.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	FCompletionFutureArray FutureArray;

	for (auto& GroupValue : LoadedRequiredGroupValues)
	{
		FCompletionPromise Promise;
		FutureArray.Add(Promise.GetFuture());

		SwitchContainerExecutionQueue.Async([this, &InCookedData, &GroupValue, Promise = MoveTemp(Promise)]() mutable
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading GroupValue %s for Event %s (%" PRIu32 ")"),
				*GroupValue.GroupValueCookedData.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

			UnloadGroupValueResources(GroupValue.LoadedData, GroupValue.GroupValueCookedData);
			Promise.EmplaceValue();
		});
	}

	// Unload Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	if (bLoadedSwitchContainerLeaves) for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		FCompletionPromise Promise;
		FutureArray.Add(Promise.GetFuture());

		SwitchContainerExecutionQueue.Async([this, &SwitchContainerLeaf, &InCookedData, Promise = MoveTemp(Promise)]() mutable
		{
			FWwiseSwitchContainerLeafGroupValueUsageCount* UsageCount = nullptr;
			for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Removing requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
					*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

				FWwiseSwitchContainerLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue));
				if (UNLIKELY(!Info))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not find requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
						*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
					continue;
				}

				if (!UsageCount)
				{
					for (auto* Leaf : Info->Leaves)
					{
						if (&Leaf->Key == &SwitchContainerLeaf)
						{
							UsageCount = Leaf;
							break;
						}
					}

					if (UNLIKELY(!UsageCount))
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not find requested Leaf in GroupValue %s in Event %s (%" PRIu32 ")"),
							*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
						continue;
					}
				}
				Info->Leaves.Remove(UsageCount);

				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadCount: %d"), Info->LoadCount);
				if (Info->ShouldBeLoaded())
				{
					UsageCount->LoadedGroupValues.Remove(GroupValue);
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), (int)UsageCount->LoadedGroupValues.Num());
				}
			}

			if (LIKELY(UsageCount))
			{
				if (UNLIKELY(UsageCount->LoadedGroupValues.Num() > 0))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("There are still %d loaded elements for leaf in Event %s (%" PRIu32 ")"),
						(int)UsageCount->LoadedGroupValues.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
				}
				UnloadSwitchContainerLeafResources(UsageCount->LoadedData, UsageCount->Key);
				delete UsageCount;
			}
			Promise.EmplaceValue();
		});
	}

	WaitForFutures(FutureArray);
	LoadedRequiredGroupValues.Empty();
	bLoadedSwitchContainerLeaves = false;
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceResources(FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading External Source %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);

	if (LoadedData.IsLoaded())
	{
		FCompletionPromise Promise;
		auto Future = Promise.GetFuture();
		UnloadExternalSourceFile(InCookedData, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
		Future.Wait();
		LoadedData.bLoaded = false;
	}
}

void FWwiseResourceLoaderImpl::UnloadGroupValueResources(FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading GroupValue %s (%d %" PRIu32 " %" PRIu32 ")"),
		*InCookedData.DebugName.ToString(), (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);

	FWwiseSwitchContainerLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData));
	if (UNLIKELY(!Info))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not find requested GroupValue %s (%d %" PRIu32 " %" PRIu32 ")"),
			*InCookedData.DebugName.ToString(), (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);
		return;
	}
	check(Info->ShouldBeLoaded());
	--Info->LoadCount;

	if (!Info->ShouldBeLoaded())
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Last GroupValue %s (%d %" PRIu32 " %" PRIu32 ") unload. Unloading %d leaves."),
			*InCookedData.DebugName.ToString(), (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->Leaves.Num());

		for (auto* UsageCount : Info->Leaves)
		{
			check(UsageCount->LoadedGroupValues.Num() > 0);
			check(UsageCount->LoadedData.IsLoaded() && UsageCount->HaveAllKeys() || !UsageCount->LoadedData.IsLoaded());

			UsageCount->LoadedGroupValues.Remove(InCookedData);
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), (int)UsageCount->LoadedGroupValues.Num());
			UnloadSwitchContainerLeafResources(UsageCount->LoadedData, UsageCount->Key);
		}
	}
	else
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("GroupValue %s (%d %" PRIu32 " %" PRIu32 ") still loaded (Count: %d times)."),
			*InCookedData.DebugName.ToString(), (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->LoadCount);
	}
}

void FWwiseResourceLoaderImpl::UnloadInitBankResources(FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Init SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);

	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (LoadedData.bLoaded)
	{
		FCompletionPromise Promise;
		auto Future = Promise.GetFuture();
		UnloadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
		Future.Wait();
		LoadedData.bLoaded = false;
	}

	FCompletionFutureArray FutureArray;
	AddUnloadMediaFutures(FutureArray, LoadedMedia);
	WaitForFutures(FutureArray);
}

void FWwiseResourceLoaderImpl::UnloadMediaResources(FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Media %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);

	if (LoadedData.IsLoaded())
	{
		FCompletionPromise Promise;
		auto Future = Promise.GetFuture();
		UnloadMediaFile(InCookedData, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
		Future.Wait();
		LoadedData.bLoaded = false;
	}
}

void FWwiseResourceLoaderImpl::UnloadShareSetResources(FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading ShareSet %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	FCompletionFutureArray FutureArray;

	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks);
	WaitForFutures(FutureArray);
	AddUnloadMediaFutures(FutureArray, LoadedMedia);
	WaitForFutures(FutureArray);
}

void FWwiseResourceLoaderImpl::UnloadSoundBankResources(FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);

	if (LoadedData.IsLoaded())
	{
		FCompletionPromise Promise;
		auto Future = Promise.GetFuture();
		UnloadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
		Future.Wait();
		LoadedData.bLoaded = false;
	}
}

void FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources(FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData& LoadedData, const FWwiseSwitchContainerLeafCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading Switch Container Leaf"));

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	FCompletionFutureArray FutureArray;

	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks);
	WaitForFutures(FutureArray);

	AddUnloadExternalSourceFutures(FutureArray, LoadedExternalSources);
	AddUnloadMediaFutures(FutureArray, LoadedMedia);
	WaitForFutures(FutureArray);

	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);
}


void FWwiseResourceLoaderImpl::AttachAuxBusNode(FWwiseLoadedAuxBus AuxBusListNode)
{
	LoadedAuxBusList.AddTail(AuxBusListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
}

void FWwiseResourceLoaderImpl::AttachEventNode(FWwiseLoadedEvent EventListNode)
{
	LoadedEventList.AddTail(EventListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
}

void FWwiseResourceLoaderImpl::AttachExternalSourceNode(FWwiseLoadedExternalSource ExternalSourceListNode)
{
	LoadedExternalSourceList.AddTail(ExternalSourceListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
}

void FWwiseResourceLoaderImpl::AttachGroupValueNode(FWwiseLoadedGroupValue GroupValueListNode)
{
	LoadedGroupValueList.AddTail(GroupValueListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
}

void FWwiseResourceLoaderImpl::AttachInitBankNode(FWwiseLoadedInitBank InitBankListNode)
{
	LoadedInitBankList.AddTail(InitBankListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
}

void FWwiseResourceLoaderImpl::AttachMediaNode(FWwiseLoadedMedia MediaListNode)
{
	LoadedMediaList.AddTail(MediaListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
}

void FWwiseResourceLoaderImpl::AttachShareSetNode(FWwiseLoadedShareSet ShareSetListNode)
{
	LoadedShareSetList.AddTail(ShareSetListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderShareSets);
}

void FWwiseResourceLoaderImpl::AttachSoundBankNode(FWwiseLoadedSoundBank SoundBankListNode)
{
	LoadedSoundBankList.AddTail(SoundBankListNode);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
}


void FWwiseResourceLoaderImpl::DetachAuxBusNode(FWwiseLoadedAuxBus AuxBusListNode)
{
	LoadedAuxBusList.RemoveNode(AuxBusListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
}

void FWwiseResourceLoaderImpl::DetachEventNode(FWwiseLoadedEvent EventListNode)
{
	LoadedEventList.RemoveNode(EventListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
}

void FWwiseResourceLoaderImpl::DetachExternalSourceNode(FWwiseLoadedExternalSource ExternalSourceListNode)
{
	LoadedExternalSourceList.RemoveNode(ExternalSourceListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
}

void FWwiseResourceLoaderImpl::DetachGroupValueNode(FWwiseLoadedGroupValue GroupValueListNode)
{
	LoadedGroupValueList.RemoveNode(GroupValueListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
}

void FWwiseResourceLoaderImpl::DetachInitBankNode(FWwiseLoadedInitBank InitBankListNode)
{
	LoadedInitBankList.RemoveNode(InitBankListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
}

void FWwiseResourceLoaderImpl::DetachMediaNode(FWwiseLoadedMedia MediaListNode)
{
	LoadedMediaList.RemoveNode(MediaListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
}

void FWwiseResourceLoaderImpl::DetachShareSetNode(FWwiseLoadedShareSet ShareSetListNode)
{
	LoadedShareSetList.RemoveNode(ShareSetListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderShareSets);
}

void FWwiseResourceLoaderImpl::DetachSoundBankNode(FWwiseLoadedSoundBank SoundBankListNode)
{
	LoadedSoundBankList.RemoveNode(SoundBankListNode, false);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
}


void FWwiseResourceLoaderImpl::AddLoadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
                                                            const TArray<FWwiseExternalSourceCookedData>& InExternalSources, const TCHAR* InType, FName InDebugName, uint32 InShortId) const
{
	for (const auto& ExternalSource : InExternalSources)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadExternalSourceFile(ExternalSource, [this, &ExternalSource, &LoadedExternalSources, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_CLOG(InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load External Source %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*ExternalSource.DebugName.ToString(), (uint32)ExternalSource.Cookie,
					InType, *InDebugName.ToString(), (uint32)InShortId);
				UE_CLOG(!InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load External Source %s (%" PRIu32 ") for %s"),
					InType,
					*ExternalSource.DebugName.ToString(), (uint32)ExternalSource.Cookie,
					InType);
				Promise.EmplaceValue();
			}
			else
			{
				FileExecutionQueue.Async([&ExternalSource, &LoadedExternalSources, Promise = MoveTemp(Promise)]() mutable
				{
					LoadedExternalSources.Add(&ExternalSource);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadExternalSourceFutures(FCompletionFutureArray& FutureArray,
	TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources) const
{
	for (const auto* ExternalSource : LoadedExternalSources)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadExternalSourceFile(*ExternalSource, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
	LoadedExternalSources.Empty();
}

void FWwiseResourceLoaderImpl::AddLoadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia,
                                                   const TArray<FWwiseMediaCookedData>& InMedia, const TCHAR* InType, FName InDebugName, uint32 InShortId) const
{
	for (const auto& Media : InMedia)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadMediaFile(Media, [this, &Media, &LoadedMedia, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_CLOG(InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load Media %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*Media.DebugName.ToString(), (uint32)Media.MediaId,
					InType, *InDebugName.ToString(), (uint32)InShortId);
				UE_CLOG(!InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load Media %s (%" PRIu32 ") for %s"),
					InType,
					*Media.DebugName.ToString(), (uint32)Media.MediaId,
					InType);
				Promise.EmplaceValue();
			}
			else
			{
				FileExecutionQueue.Async([&Media, &LoadedMedia, Promise = MoveTemp(Promise)]() mutable
				{
					LoadedMedia.Add(&Media);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadMediaFutures(FCompletionFutureArray& FutureArray,
	TArray<const FWwiseMediaCookedData*>& LoadedMedia) const
{
	for (const auto* Media : LoadedMedia)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadMediaFile(*Media, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
	LoadedMedia.Empty();
}

void FWwiseResourceLoaderImpl::AddLoadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
                                                       const TArray<FWwiseSoundBankCookedData>& InSoundBank, const TCHAR* InType, FName InDebugName, uint32 InShortId) const
{
	for (const auto& SoundBank : InSoundBank)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadSoundBankFile(SoundBank, [this, &SoundBank, &LoadedSoundBanks, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_CLOG(InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load SoundBank %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*SoundBank.DebugName.ToString(), (uint32)SoundBank.SoundBankId,
					InType, *InDebugName.ToString(), (uint32)InShortId);
				UE_CLOG(!InDebugName.IsValid(), LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load SoundBank %s (%" PRIu32 ") for %s"),
					InType,
					*SoundBank.DebugName.ToString(), (uint32)SoundBank.SoundBankId,
					InType);
				Promise.EmplaceValue();
			}
			else
			{
				FileExecutionQueue.Async([&SoundBank, &LoadedSoundBanks, Promise = MoveTemp(Promise)]() mutable
				{
					LoadedSoundBanks.Add(&SoundBank);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadSoundBankFutures(FCompletionFutureArray& FutureArray,
	TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks) const
{
	for (const auto* SoundBank : LoadedSoundBanks)
	{
		TPromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadSoundBankFile(*SoundBank, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
	LoadedSoundBanks.Empty();
}

void FWwiseResourceLoaderImpl::WaitForFutures(FCompletionFutureArray& FutureArray) const
{
	for (auto& Future : FutureArray)
	{
		Future.Wait();
	}
	FutureArray.Empty();

}

void FWwiseResourceLoaderImpl::LoadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FLoadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadSoundBankAsync: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString(), *InSoundBank.SoundBankPathName.ToString());

	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (UNLIKELY(!SoundBankManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));
		InCallback(false);
		return;
	}
	SoundBankManager->LoadSoundBank(InSoundBank, GetUnrealPath(), [&InSoundBank, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadSoundBankAsync: %" PRIu32 "] %s: Done."),
			(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FUnloadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InSoundBank.SoundBankPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadSoundBankAsync: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString(), *InSoundBank.SoundBankPathName.ToString());

	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (UNLIKELY(!SoundBankManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));
		InCallback();
		return;
	}
	SoundBankManager->UnloadSoundBank(InSoundBank, GetUnrealPath(), [&InSoundBank, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadSoundBankAsync: %" PRIu32 "] %s: Done."),
			(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString());
		InCallback();
	});
}

void FWwiseResourceLoaderImpl::LoadMediaFile(const FWwiseMediaCookedData& InMedia, FLoadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadMediaAsync: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName.ToString(), *InMedia.MediaPathName.ToString());

	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));
		InCallback(false);
		return;
	}

	MediaManager->LoadMedia(InMedia, GetUnrealPath(), [&InMedia, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadMediaAsync: %" PRIu32 "] %s: Done."),
			(uint32)InMedia.MediaId, *InMedia.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadMediaFile(const FWwiseMediaCookedData& InMedia, FUnloadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadMediaAsync: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName.ToString(), *InMedia.MediaPathName.ToString());


	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));
		InCallback();
		return;
	}

	MediaManager->UnloadMedia(InMedia, GetUnrealPath(), [&InMedia, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadMediaAsync: %" PRIu32 "] %s: Done."),
			(uint32)InMedia.MediaId, *InMedia.DebugName.ToString());
		InCallback();
	});
}

void FWwiseResourceLoaderImpl::LoadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FLoadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadExternalSourceAsync: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());

	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));
		InCallback(false);
		return;
	}

	ExternalSourceManager->LoadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&InExternalSource, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadExternalSourceAsync: %" PRIu32 "] %s: Done."),
			(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FUnloadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadExternalSourceAsync: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());

	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));
		InCallback();
		return;
	}

	ExternalSourceManager->UnloadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&InExternalSource, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadExternalSourceAsync: %" PRIu32 "] %s: Done."),
			(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());
		InCallback();
	});
}
