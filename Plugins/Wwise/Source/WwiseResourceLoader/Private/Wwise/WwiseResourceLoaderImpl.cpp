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

#include "Wwise/WwiseResourceLoaderImpl.h"

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSharesetCookedData.h"

#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"

#include <inttypes.h>


UWwiseResourceLoaderImpl::UWwiseResourceLoaderImpl()
{
}

FString UWwiseResourceLoaderImpl::GetUnrealExternalSourcePath() const
{
#if WITH_EDITORONLY_DATA
	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks / CurrentPlatform.Platform->ExternalSourceRootPath;
#else
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealExternalSourcePath: Failed to retrieve External Source Manager, returning empty string."));
		return {};
	}
	return FPaths::ProjectContentDir() / ExternalSourceManager->GetStagingDirectory();
#endif
}

FString UWwiseResourceLoaderImpl::GetUnrealPath() const
{
#if WITH_EDITOR
	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks;
#elif WITH_EDITORONLY_DATA
	UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealPath should not be used in WITH_EDITORONLY_DATA (Getting path for %s)"), *InPath);
	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks;
#else
	return StagePath;
#endif
}

FString UWwiseResourceLoaderImpl::GetUnrealPath(const FString& InPath) const
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

FString UWwiseResourceLoaderImpl::GetUnrealStagePath(const FString& InPath) const
{
	if (UNLIKELY(StagePath.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("StagePath not set up (GetUnrealStagePath for %s)"), *InPath);
	}
	return StagePath / InPath;
}

#if WITH_EDITORONLY_DATA
FString UWwiseResourceLoaderImpl::GetUnrealGeneratedSoundBanksPath(const FString& InPath) const
{
	if (UNLIKELY(GeneratedSoundBanksPath.Path.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("GeneratedSoundBanksPath not set up (GetUnrealGeneratedSoundBanksPath for %s)"), *InPath);
	}

	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks / InPath;
}
#endif

bool UWwiseResourceLoaderImpl::DefaultLoadSoundBankOperation(const FWwiseSoundBankCookedData& InSoundBank)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadSoundBank: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName, *InSoundBank.SoundBankPathName);

	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (UNLIKELY(!SoundBankManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));

		return false;
	}
	FEventRef WaitForDone;
	bool bResult = false;
	SoundBankManager->LoadSoundBank(InSoundBank, GetUnrealPath(), [&WaitForDone, &bResult](bool bInResult)
	{
		bResult = bInResult;
		WaitForDone->Trigger();
	});
	WaitForDone->Wait();
	return bResult;
}

bool UWwiseResourceLoaderImpl::DefaultUnloadSoundBankOperation(const FWwiseSoundBankCookedData& InSoundBank)
{
	auto Path = GetUnrealPath(InSoundBank.SoundBankPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadSoundBank: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName, *InSoundBank.SoundBankPathName);

	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (UNLIKELY(!SoundBankManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));

		return false;
	}
	FEventRef WaitForDone;
	SoundBankManager->UnloadSoundBank(InSoundBank, GetUnrealPath(), [&WaitForDone]() { WaitForDone->Trigger(); });
	WaitForDone->Wait();

	return true;
}

bool UWwiseResourceLoaderImpl::DefaultLoadMediaOperation(const FWwiseMediaCookedData& InMedia)
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadMedia: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName, *InMedia.MediaPathName);

	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));

		return false;
	}

	FEventRef WaitForDone;
	bool bResult = false;
	MediaManager->LoadMedia(InMedia, GetUnrealPath(), [&WaitForDone, &bResult](bool bInResult)
	{
		bResult = bInResult;
		WaitForDone->Trigger();
	});
	WaitForDone->Wait();
	return bResult;
}

bool UWwiseResourceLoaderImpl::DefaultUnloadMediaOperation(const FWwiseMediaCookedData& InMedia)
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadMedia: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName, *InMedia.MediaPathName);


	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));

		return false;
	}

	FEventRef WaitForDone;
	MediaManager->UnloadMedia(InMedia, GetUnrealPath(), [&WaitForDone]() { WaitForDone->Trigger(); });
	WaitForDone->Wait();

	return true;
}

bool UWwiseResourceLoaderImpl::DefaultLoadExternalSourceOperation(const FWwiseExternalSourceCookedData& InExternalSource)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadExternalSource: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName);

	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));

		return false;
	}
	FEventRef WaitForDone;
	bool bResult = false;
	ExternalSourceManager->LoadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&WaitForDone, &bResult](bool bInResult)
	{
		bResult = bInResult;
		WaitForDone->Trigger();
	});
	WaitForDone->Wait();
	return bResult;
}

bool UWwiseResourceLoaderImpl::DefaultUnloadExternalSourceOperation(const FWwiseExternalSourceCookedData& InExternalSource)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadExternalSource: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName);

	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));

		return false;
	}
	FEventRef WaitForDone;
	ExternalSourceManager->UnloadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&WaitForDone]() { WaitForDone->Trigger(); });
	WaitForDone->Wait();

	return true;
}

void UWwiseResourceLoaderImpl::SetLanguage(const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);

	const auto OldLanguage = CurrentLanguage;
	const auto NewLanguage = InLanguage;

	if (OldLanguage == NewLanguage)
	{
		return;
	}

	UE_CLOG(OldLanguage.GetLanguageName().IsEmpty(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] To %s"), *NewLanguage.GetLanguageName());
	UE_CLOG(!OldLanguage.GetLanguageName().IsEmpty(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] from %s to %s"), *OldLanguage.GetLanguageName(), *NewLanguage.GetLanguageName());


	if (InReloadLanguage == EWwiseReloadLanguage::Safe)
	{
		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Stopping all sounds"));
		auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
		if (UNLIKELY(!SoundEngine))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: SoundEngine not available to stop all sounds"));
		}
		else
		{
			SoundEngine->StopAll();
		}
	}

	CurrentLanguage = NewLanguage;

	if (InReloadLanguage != EWwiseReloadLanguage::Manual)
	{
		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Immediately switching languages. Unloading old language %s."),
			*OldLanguage.GetLanguageName());

		for (auto& LoadedSoundBank : LoadedSoundBankList)
		{
			if (LoadedSoundBank.LanguageRef != OldLanguage)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping SoundBank %s with language %s"),
					*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, *LoadedSoundBank.LanguageRef.GetLanguageName());
				continue;
			}

			if (LoadedSoundBank.bLoaded)
			{
				auto* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
				if (LIKELY(SoundBank))
				{
					UnloadSoundBankResources(*SoundBank);
					LoadedSoundBank.bLoaded = false;
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, *LoadedSoundBank.LanguageRef.GetLanguageName());
				}
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("SetLanguage: SoundBank %s was initially not loaded. Will try to load in language %s"),
					*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, *LoadedSoundBank.LanguageRef.GetLanguageName());
			}
		}

		for (auto& LoadedAuxBus : LoadedAuxBusList)
		{
			if (LoadedAuxBus.LanguageRef != OldLanguage)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping AuxBus %s with language %s"),
					*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, *LoadedAuxBus.LanguageRef.GetLanguageName());
				continue;
			}

			if (LoadedAuxBus.bLoaded)
			{
				auto* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
				if (LIKELY(AuxBus))
				{
					UnloadAuxBusResources(*AuxBus);
					LoadedAuxBus.bLoaded = false;
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, *LoadedAuxBus.LanguageRef.GetLanguageName());
				}
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("SetLanguage: AuxBus %s was initially not loaded. Will try to load in language %s"),
					*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, *LoadedAuxBus.LanguageRef.GetLanguageName());
			}
		}

		for (auto& LoadedShareset : LoadedSharesetList)
		{
			if (LoadedShareset.LanguageRef != OldLanguage)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping Shareset %s with language %s"),
					*LoadedShareset.LocalizedSharesetCookedData.DebugName, *LoadedShareset.LanguageRef.GetLanguageName());
				continue;
			}

			if (LoadedShareset.bLoaded)
			{
				auto* Shareset = LoadedShareset.LocalizedSharesetCookedData.SharesetLanguageMap.Find(LoadedShareset.LanguageRef);
				if (LIKELY(Shareset))
				{
					UnloadSharesetResources(*Shareset);
					LoadedShareset.bLoaded = false;
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Shareset %s with language %s"),
						*LoadedShareset.LocalizedSharesetCookedData.DebugName, *LoadedShareset.LanguageRef.GetLanguageName());
				}
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("SetLanguage: Shareset %s was initially not loaded. Will try to load in language %s"),
					*LoadedShareset.LocalizedSharesetCookedData.DebugName, *LoadedShareset.LanguageRef.GetLanguageName());
			}
		}

		for (auto& LoadedEvent : LoadedEventList)
		{
			if (LoadedEvent.LanguageRef != OldLanguage)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping Event %s with language %s"),
					*LoadedEvent.LocalizedEventCookedData.DebugName, *LoadedEvent.LanguageRef.GetLanguageName());
				continue;
			}

			if (LoadedEvent.bLoaded)
			{
				auto* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
				if (LIKELY(Event))
				{
					UnloadEventResources(*Event);
					LoadedEvent.bLoaded = false;
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName, *LoadedEvent.LanguageRef.GetLanguageName());
				}
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("SetLanguage: Event %s was initially not loaded. Will try to load in language %s"),
					*LoadedEvent.LocalizedEventCookedData.DebugName, *LoadedEvent.LanguageRef.GetLanguageName());
			}
		}

		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Loading new language %s."),
			*NewLanguage.GetLanguageName());

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
				LoadedSoundBank.bLoaded = LoadSoundBankResources(*SoundBank);
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
					*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, *LoadedSoundBank.LanguageRef.GetLanguageName());
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
				LoadedAuxBus.bLoaded = LoadAuxBusResources(*AuxBus);
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
					*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, *LoadedAuxBus.LanguageRef.GetLanguageName());
			}
		}

		for (auto& LoadedShareset : LoadedSharesetList)
		{
			if (LoadedShareset.LanguageRef != OldLanguage)
			{
				continue;
			}

			LoadedShareset.LanguageRef = NewLanguage;
			auto* Shareset = LoadedShareset.LocalizedSharesetCookedData.SharesetLanguageMap.Find(LoadedShareset.LanguageRef);
			if (LIKELY(Shareset))
			{
				LoadedShareset.bLoaded = LoadSharesetResources(*Shareset);
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Shareset %s with language %s"),
					*LoadedShareset.LocalizedSharesetCookedData.DebugName, *LoadedShareset.LanguageRef.GetLanguageName());
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
				LoadedEvent.bLoaded = LoadEventResources(*Event);;
			}
			else
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
					*LoadedEvent.LocalizedEventCookedData.DebugName, *LoadedEvent.LanguageRef.GetLanguageName());
			}
		}
	}

	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Done"));
}

void UWwiseResourceLoaderImpl::SetPlatform(const FWwiseSharedPlatformId& InPlatform)
{
	UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetPlatform: Updating platform from %s (%s) to %s (%s)."),
		*CurrentPlatform.GetPlatformName(), *CurrentPlatform.GetPlatformGuid().ToString(),
		*InPlatform.GetPlatformName(), *InPlatform.GetPlatformGuid().ToString());

	CurrentPlatform = InPlatform;
}

//
// AuxBus resource handling
//

FWwiseLoadedAuxBusListNode* UWwiseResourceLoaderImpl::LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading AuxBus %s"), *InAuxBusCookedData.DebugName);

	const auto* LanguageKey = GetLanguageMapKey(InAuxBusCookedData.AuxBusLanguageMap, InLanguageOverride, InAuxBusCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		return nullptr;
	}

	const auto* AuxBus = InAuxBusCookedData.AuxBusLanguageMap.Find(*LanguageKey);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBus: Could not find AuxBus %s in language %" PRIu32),
			*InAuxBusCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}
	if (UNLIKELY(!LoadAuxBusResources(*AuxBus)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBus: Could not load AuxBus %s in language %" PRIu32),
			*InAuxBusCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	auto LoadedAuxBus = FWwiseLoadedAuxBus(InAuxBusCookedData, *LanguageKey);
	auto* LoadedNode = new FWwiseLoadedAuxBusListNode(LoadedAuxBus);
	LoadedAuxBusList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedAuxBusList: %d] AuxBus %s in language %" PRIu32),
		LoadedAuxBusList.Num(), *InAuxBusCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
	INC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadAuxBusResources(const FWwiseAuxBusCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading AuxBus %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.AuxBusId);

	TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
	TArray<const FWwiseMediaCookedData*> LoadedMedia;
	bool bResult = true;

	if (LIKELY(bResult)) for (const auto& Media : InCookedData.Media)
	{
		if (UNLIKELY(!LoadMediaResources(Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusResources: Could not load Media %s (%" PRIu32 ") for AuxBus %s (%" PRIu32 ")"),
				*Media.DebugName, (uint32)Media.MediaId,
				*InCookedData.DebugName, (uint32)InCookedData.AuxBusId);

			bResult = false;
			break;
		}
		LoadedMedia.Add(&Media);
	}

	if (LIKELY(bResult)) for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		if (UNLIKELY(!LoadSoundBankResources(SoundBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusResources: Could not load SoundBank %s (%" PRIu32 ") for AuxBus %s (%" PRIu32 ")"),
				*SoundBank.DebugName, (uint32)SoundBank.SoundBankId,
				*InCookedData.DebugName, (uint32)InCookedData.AuxBusId);

			bResult = false;
			break;
		}
		LoadedSoundBanks.Add(&SoundBank);
	}

	if (UNLIKELY(!bResult))
	{
		for (const auto* SoundBank : LoadedSoundBanks)
		{
			UnloadSoundBankResources(*SoundBank);
		}
		for (const auto& Media : LoadedMedia)
		{
			UnloadMediaResources(*Media);
		}
	}
	return bResult;
}

void UWwiseResourceLoaderImpl::UnloadAuxBus(FWwiseLoadedAuxBusListNode* InAuxBusListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedAuxBus = InAuxBusListNode->GetValue();
	const auto* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadAuxBus: Could not find AuxBus %s in language %" PRIu32),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, (uint32)LoadedAuxBus.LanguageRef.GetLanguageId());
		return;
	}

	UnloadAuxBusResources(*AuxBus);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedAuxBusList: %d] AuxBus %s in language %" PRIu32),
		LoadedAuxBusList.Num() - 1, *LoadedAuxBus.LocalizedAuxBusCookedData.DebugName, (uint32)LoadedAuxBus.LanguageRef.GetLanguageId());
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
	LoadedAuxBusList.RemoveNode(InAuxBusListNode);
}

void UWwiseResourceLoaderImpl::UnloadAuxBusResources(const FWwiseAuxBusCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading AuxBus %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.AuxBusId);

	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		UnloadSoundBankResources(SoundBank);
	}
	for (const auto& Media : InCookedData.Media)
	{
		UnloadMediaResources(Media);
	}
}

//
// Event resource handling
//

FWwiseLoadedEventListNode* UWwiseResourceLoaderImpl::LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Event %s"), *InEventCookedData.DebugName);

	const auto* LanguageKey = GetLanguageMapKey(InEventCookedData.EventLanguageMap, InLanguageOverride, InEventCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEvent: Could not find language for Event %s"), *InEventCookedData.DebugName);
		return nullptr;
	}

	const auto* Event = InEventCookedData.EventLanguageMap.Find(*LanguageKey);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEvent: Could not find Event %s in language %" PRIu32),
			*InEventCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	if (UNLIKELY(!LoadEventResources(*Event)))
	{
		return nullptr;
	}

	auto LoadedEvent = FWwiseLoadedEvent(InEventCookedData, *LanguageKey);
	auto* LoadedNode = new FWwiseLoadedEventListNode(LoadedEvent);
	LoadedEventList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedEventList: %d] Event %s in language %" PRIu32),
		LoadedEventList.Num(), *InEventCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
	INC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadEventResources(const FWwiseEventCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Event %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.EventId);

	TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
	TArray<const FWwiseMediaCookedData*> LoadedMedia;
	TArray<const FWwiseExternalSourceCookedData*> LoadedExternalSources;
	bool bResult = true;

	if (LIKELY(bResult)) for (const auto& Media : InCookedData.Media)
	{
		if (UNLIKELY(!LoadMediaResources(Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load Media %s (%" PRIu32 ") for Event %s (%" PRIu32 ")"),
				*Media.DebugName, (uint32)Media.MediaId,
				*InCookedData.DebugName, (uint32)InCookedData.EventId);

			bResult = false;
			break;
		}
		LoadedMedia.Add(&Media);
	}

	if (LIKELY(bResult)) for (const auto& ExternalSource : InCookedData.ExternalSources)
	{
		if (UNLIKELY(!LoadExternalSourceResources(ExternalSource)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load External Source %s (%" PRIu32 ") for Event %s (%" PRIu32 ")"),
				*ExternalSource.DebugName, (uint32)ExternalSource.Cookie,
				*InCookedData.DebugName, (uint32)InCookedData.EventId);

			bResult = false;
			break;
		}
		LoadedExternalSources.Add(&ExternalSource);
	}

	if (LIKELY(bResult)) for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		if (UNLIKELY(!LoadSoundBankResources(SoundBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load SoundBank %s (%" PRIu32 ") for Event %s (%" PRIu32 ")"),
				*SoundBank.DebugName, (uint32)SoundBank.SoundBankId,
				*InCookedData.DebugName, (uint32)InCookedData.EventId);

			bResult = false;
			break;
		}
		LoadedSoundBanks.Add(&SoundBank);
	}

	if (LIKELY(bResult) && UNLIKELY(!LoadEventSwitchContainerResources(InCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load switches for Event %s (%" PRIu32 ")"),
			*InCookedData.DebugName, (uint32)InCookedData.EventId);
		bResult = false;
	}

	if (UNLIKELY(!bResult))
	{
		for (const auto* SoundBank : LoadedSoundBanks)
		{
			UnloadSoundBankResources(*SoundBank);
		}
		for (const auto* Media : LoadedMedia)
		{
			UnloadMediaResources(*Media);
		}
		for (const auto* ExternalSource : LoadedExternalSources)
		{
			UnloadExternalSourceResources(*ExternalSource);
		}
	}
	return bResult;
}

bool UWwiseResourceLoaderImpl::LoadEventSwitchContainerResources(const FWwiseEventCookedData& InCookedData)
{
	// Load required GroupValues
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)InCookedData.RequiredGroupValueSet.Num(), *InCookedData.DebugName, (uint32)InCookedData.EventId);

	TArray<FWwiseLoadedGroupValueListNode*> LoadedRequiredGroupValues;
	for (const auto& GroupValue : InCookedData.RequiredGroupValueSet)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading GroupValue %s for Event %s (%" PRIu32 ")"),
			*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);
		if (UNLIKELY(!LoadGroupValueResources(GroupValue)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not load required GroupValue %s for Event %s (%" PRIu32 ")"),
				*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);
			// Not returning false. Event is still partially loaded.
			continue;
		}
		auto LoadedGroupValue = FWwiseLoadedGroupValue(GroupValue);
		auto* LoadedNode = new FWwiseLoadedGroupValueListNode(LoadedGroupValue);
		LoadedGroupValueList.AddTail(LoadedNode);
		LoadedRequiredGroupValues.Add(LoadedNode);
	}
	LoadedEventRequiredGroupValueNodes.Add(InCookedData.EventId, LoadedRequiredGroupValues);

	// Load Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName, (uint32)InCookedData.EventId);

	for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		check(SwitchContainerLeaf.GroupValueSet.Num() > 0);
		FWwiseSwitchContainerLeafGroupValueUsageCount* UsageCount = new FWwiseSwitchContainerLeafGroupValueUsageCount(SwitchContainerLeaf);
		for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Adding requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
				*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);

			auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseLoadedGroupValueInfo(GroupValue));
			auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseLoadedGroupValueInfo(GroupValue), nullptr);
			FWwiseLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
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
					check(!UsageCount->bLoaded);
					UsageCount->bLoaded = LoadSwitchContainerLeafResources(SwitchContainerLeaf);
				}
			}
		}
	}

	return true;
}

void UWwiseResourceLoaderImpl::UnloadEvent(FWwiseLoadedEventListNode* InEventListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedEvent = InEventListNode->GetValue();
	const auto* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEvent: Could not find Event %s in language %" PRIu32),
			*LoadedEvent.LocalizedEventCookedData.DebugName, (uint32)LoadedEvent.LanguageRef.GetLanguageId());
		return;
	}

	UnloadEventResources(*Event);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedEventList: %d] Event %s in language %" PRIu32),
		LoadedEventList.Num() - 1, *LoadedEvent.LocalizedEventCookedData.DebugName, (uint32)LoadedEvent.LanguageRef.GetLanguageId());
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
	LoadedEventList.RemoveNode(InEventListNode);
}

void UWwiseResourceLoaderImpl::UnloadEventResources(const FWwiseEventCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Event %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.EventId);

	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		UnloadSoundBankResources(SoundBank);
	}
	for (const auto& Media : InCookedData.Media)
	{
		UnloadMediaResources(Media);
	}
	for (const auto& ExternalSource : InCookedData.ExternalSources)
	{
		UnloadExternalSourceResources(ExternalSource);
	}

	UnloadEventSwitchContainerResources(InCookedData);
}

bool UWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources(const FWwiseEventCookedData& InCookedData)
{
	// Unload required GroupValues
	TArray<FWwiseLoadedGroupValueListNode*> LoadedRequiredGroupValues;
	LoadedEventRequiredGroupValueNodes.RemoveAndCopyValue(InCookedData.EventId, LoadedRequiredGroupValues);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)LoadedRequiredGroupValues.Num(), *InCookedData.DebugName, (uint32)InCookedData.EventId);

	for (auto* GroupValueNode : LoadedRequiredGroupValues)
	{
		FWwiseLoadedGroupValue& LoadedGroupValue = GroupValueNode->GetValue();
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading GroupValue %s for Event %s (%" PRIu32 ")"),
			*LoadedGroupValue.GroupValueCookedData.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);

		UnloadGroupValueResources(LoadedGroupValue.GroupValueCookedData);

		LoadedGroupValueList.RemoveNode(GroupValueNode);
	}

	// Unload Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName, (uint32)InCookedData.EventId);

	for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		FWwiseSwitchContainerLeafGroupValueUsageCount* UsageCount = nullptr;
		for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Removing requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
				*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);

			FWwiseLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(FWwiseLoadedGroupValueInfo(GroupValue));
			if (UNLIKELY(!Info))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not find requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
					*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);
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
						*GroupValue.DebugName, *InCookedData.DebugName, (uint32)InCookedData.EventId);
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
					(int)UsageCount->LoadedGroupValues.Num(), *InCookedData.DebugName, (uint32)InCookedData.EventId);
			}
			if (UsageCount->bLoaded)
			{
				UnloadSwitchContainerLeafResources(SwitchContainerLeaf);
			}
			delete UsageCount;
		}
	}

	return true;
}

//
// External Source resource handling
//


FWwiseLoadedExternalSourceListNode* UWwiseResourceLoaderImpl::LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading External Source %s"), *InExternalSourceCookedData.DebugName);

	if (UNLIKELY(!LoadExternalSourceResources(InExternalSourceCookedData)))
	{
		return nullptr;
	}

	auto LoadedExternalSource = FWwiseLoadedExternalSource(InExternalSourceCookedData);
	auto* LoadedNode = new FWwiseLoadedExternalSourceListNode(LoadedExternalSource);
	LoadedExternalSourceList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedExternalSourceList: %d] External Source %s"),
		LoadedExternalSourceList.Num(), *InExternalSourceCookedData.DebugName);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadExternalSourceResources(const FWwiseExternalSourceCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading External Source %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.Cookie);

	if (UNLIKELY(!DefaultLoadExternalSourceOperation(InCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Log, TEXT("LoadExternalSourceResources: Could not load External Source %s (%" PRIu32 ")"),
			*InCookedData.DebugName, (uint32)InCookedData.Cookie);
		return false;
	}

	return true;
}

void UWwiseResourceLoaderImpl::UnloadExternalSource(FWwiseLoadedExternalSourceListNode* InExternalSourceListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedExternalSource = InExternalSourceListNode->GetValue();
	UnloadExternalSourceResources(LoadedExternalSource.ExternalSourceCookedData);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedExternalSourceList: %d] External Source %s"),
		LoadedExternalSourceList.Num() - 1, *LoadedExternalSource.ExternalSourceCookedData.DebugName);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
	LoadedExternalSourceList.RemoveNode(InExternalSourceListNode);
}

void UWwiseResourceLoaderImpl::UnloadExternalSourceResources(const FWwiseExternalSourceCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading External Source %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.Cookie);

	DefaultUnloadExternalSourceOperation(InCookedData);
}

//
// GroupValue resource handling
//

FWwiseLoadedGroupValueListNode* UWwiseResourceLoaderImpl::LoadGroupValue(const FWwiseGroupValueCookedData& InSwitchCookedData)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading GroupValue %s"), *InSwitchCookedData.DebugName);

	if (UNLIKELY(!LoadGroupValueResources(InSwitchCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadGroupValue: Could not load GroupValue %s"), *InSwitchCookedData.DebugName);
		return nullptr;
	}

	auto LoadedSwitch = FWwiseLoadedGroupValue(InSwitchCookedData);
	auto* LoadedNode = new FWwiseLoadedGroupValueListNode(LoadedSwitch);
	LoadedGroupValueList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedGroupValueList: %d] GroupValue %s"),
		LoadedGroupValueList.Num(), *InSwitchCookedData.DebugName);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadGroupValueResources(const FWwiseGroupValueCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading GroupValue %s (%d %" PRIu32 " %" PRIu32 ")"),
		*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);

	auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseLoadedGroupValueInfo(InCookedData));
	auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseLoadedGroupValueInfo(InCookedData), nullptr);
	FWwiseLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
	const bool bWasLoaded = Info.ShouldBeLoaded();
	++Info.LoadCount;

	if (!bWasLoaded && Info.ShouldBeLoaded())
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("First GroupValue %s (%d %" PRIu32 " %" PRIu32 ") load. Loading %d leaves."),
			*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.Leaves.Num());

		for (auto* UsageCount : Info.Leaves)
		{
			bool bIsAlreadyCreated = false;
			UsageCount->LoadedGroupValues.Add(InCookedData, &bIsAlreadyCreated);
			check(!bIsAlreadyCreated);
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), UsageCount->LoadedGroupValues.Num());
			if (UsageCount->HaveAllKeys())
			{
				check(!UsageCount->bLoaded);
				UsageCount->bLoaded = LoadSwitchContainerLeafResources(UsageCount->Key);
			}
		}
	}
	else
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("GroupValue %s (%d %" PRIu32 " %" PRIu32 ") already loaded (Count: %d times)."),
			*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.LoadCount);
	}

	return true;
}

void UWwiseResourceLoaderImpl::UnloadGroupValue(FWwiseLoadedGroupValueListNode* InSwitchListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	auto& LoadedSwitch = InSwitchListNode->GetValue();
	UnloadGroupValueResources(LoadedSwitch.GroupValueCookedData);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedGroupValueList: %d] GroupValue %s"),
		LoadedGroupValueList.Num() - 1, *LoadedSwitch.GroupValueCookedData.DebugName);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
	LoadedGroupValueList.RemoveNode(InSwitchListNode);
}

void UWwiseResourceLoaderImpl::UnloadGroupValueResources(const FWwiseGroupValueCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading GroupValue %s (%d %" PRIu32 " %" PRIu32 ")"),
		*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);

	FWwiseLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(FWwiseLoadedGroupValueInfo(InCookedData));
	if (UNLIKELY(!Info))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not find requested GroupValue %s (%d %" PRIu32 " %" PRIu32 ")"),
			*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);
		return;
	}
	check(Info->ShouldBeLoaded());
	--Info->LoadCount;

	if (!Info->ShouldBeLoaded())
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Last GroupValue %s (%d %" PRIu32 " %" PRIu32 ") unload. Unloading %d leaves."),
			*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->Leaves.Num());

		for (auto* UsageCount : Info->Leaves)
		{
			check(UsageCount->LoadedGroupValues.Num() > 0);
			check(UsageCount->bLoaded && UsageCount->HaveAllKeys() || !UsageCount->bLoaded);

			UsageCount->LoadedGroupValues.Remove(InCookedData);
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- LoadedGroupValueCount: %d"), (int)UsageCount->LoadedGroupValues.Num());
			if (UsageCount->bLoaded)
			{
				UsageCount->bLoaded = false;
				UnloadSwitchContainerLeafResources(UsageCount->Key);
			}
		}
	}
	else
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("GroupValue %s (%d %" PRIu32 " %" PRIu32 ") still loaded (Count: %d times)."),
			*InCookedData.DebugName, (int)InCookedData.Type, (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->LoadCount);
	}
}

//
// Init SoundBank resource handling
//

FWwiseLoadedInitBankListNode* UWwiseResourceLoaderImpl::LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Init SoundBank %s"), *InInitBankCookedData.DebugName);

	if (UNLIKELY(!LoadInitBankResources(InInitBankCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBank: Could not load Init SoundBank %s"),
			*InInitBankCookedData.DebugName);
		return nullptr;
	}

	auto LoadedInitBank = FWwiseLoadedInitBank(InInitBankCookedData);
	auto* LoadedNode = new FWwiseLoadedInitBankListNode(LoadedInitBank);
	LoadedInitBankList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedInitBankList: %d] Init SoundBank %s"),
		LoadedInitBankList.Num(), *InInitBankCookedData.DebugName);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadInitBankResources(const FWwiseInitBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Init SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);

	TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
	TArray<const FWwiseMediaCookedData*> LoadedMedia;
	bool bResult = true;

	if (LIKELY(bResult)) for (const auto& Media : InCookedData.Media)
	{
		if (UNLIKELY(!LoadMediaResources(Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: Could not load Media %s (%" PRIu32 ") for Init SoundBank %s (%" PRIu32 ")"),
				*Media.DebugName, (uint32)Media.MediaId,
				*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);

			bResult = false;
			break;
		}
		LoadedMedia.Add(&Media);
	}

	if (LIKELY(bResult)) if (UNLIKELY(!LoadSoundBankResources(InCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: Could not load SoundBank %s (%" PRIu32 ")"),
			*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);
		bResult = false;
	}

	if (UNLIKELY(!bResult))
	{
		UnloadSoundBankResources(InCookedData);

		for (const auto& SoundBank : LoadedSoundBanks)
		{
			UnloadSoundBankResources(*SoundBank);
		}

		for (const auto& Media : LoadedMedia)
		{
			UnloadMediaResources(*Media);
		}
	}
	return bResult;
}

void UWwiseResourceLoaderImpl::UnloadInitBank(FWwiseLoadedInitBankListNode* InInitBankListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedInitBank = InInitBankListNode->GetValue();
	UnloadInitBankResources(LoadedInitBank.InitBankCookedData);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedInitBankList: %d] Init SoundBank %s"),
		LoadedInitBankList.Num() - 1, *LoadedInitBank.InitBankCookedData.DebugName);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
	LoadedInitBankList.RemoveNode(InInitBankListNode);
}

void UWwiseResourceLoaderImpl::UnloadInitBankResources(const FWwiseInitBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Init SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);

	UnloadSoundBankResources(InCookedData);

	for (const auto& Media : InCookedData.Media)
	{
		UnloadMediaResources(Media);
	}
}

//
// Media resource handling
//

FWwiseLoadedMediaListNode* UWwiseResourceLoaderImpl::LoadMedia(const FWwiseMediaCookedData& InMediaCookedData)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Media %s"), *InMediaCookedData.DebugName);

	if (UNLIKELY(!LoadMediaResources(InMediaCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMedia: Could not load Media %s"),
			*InMediaCookedData.DebugName);
		return nullptr;
	}

	auto LoadedMedia = FWwiseLoadedMedia(InMediaCookedData);
	auto* LoadedNode = new FWwiseLoadedMediaListNode(LoadedMedia);
	LoadedMediaList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedMediaList: %d] Media %s"),
		LoadedMediaList.Num(), *InMediaCookedData.DebugName);
	INC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadMediaResources(const FWwiseMediaCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Media %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.MediaId);

	if (UNLIKELY(!DefaultLoadMediaOperation(InCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaResources: Could not load Media %s (%" PRIu32 ")"),
			*InCookedData.DebugName, (uint32)InCookedData.MediaId);
		return false;
	}

	return true;
}

void UWwiseResourceLoaderImpl::UnloadMedia(FWwiseLoadedMediaListNode* InMediaListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedMedia = InMediaListNode->GetValue();
	UnloadMediaResources(LoadedMedia.MediaCookedData);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedMediaList: %d] Media %s"),
		LoadedMediaList.Num() - 1, *LoadedMedia.MediaCookedData.DebugName);
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
	LoadedMediaList.RemoveNode(InMediaListNode);
}

void UWwiseResourceLoaderImpl::UnloadMediaResources(const FWwiseMediaCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Media %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.MediaId);

	DefaultUnloadMediaOperation(InCookedData);
}

//
// Shareset resource handling
//

FWwiseLoadedSharesetListNode* UWwiseResourceLoaderImpl::LoadShareset(const FWwiseLocalizedSharesetCookedData& InSharesetCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Shareset %s"), *InSharesetCookedData.DebugName);

	const auto* LanguageKey = GetLanguageMapKey(InSharesetCookedData.SharesetLanguageMap, InLanguageOverride, InSharesetCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareset: Could not find language for Shareset %s"), *InSharesetCookedData.DebugName);
		return nullptr;
	}

	const auto* Shareset = InSharesetCookedData.SharesetLanguageMap.Find(*LanguageKey);
	if (UNLIKELY(!Shareset))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareset: Could not find Shareset %s in language %" PRIu32),
			*InSharesetCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	if (UNLIKELY(!LoadSharesetResources(*Shareset)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareset: Could not load Shareset %s in language %" PRIu32),
			*InSharesetCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	auto LoadedShareset = FWwiseLoadedShareset(InSharesetCookedData, *LanguageKey);
	auto* LoadedNode = new FWwiseLoadedSharesetListNode(LoadedShareset);
	LoadedSharesetList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedSharesetList: %d] Shareset %s in language %" PRIu32),
		LoadedSharesetList.Num(), *InSharesetCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
	INC_DWORD_STAT(STAT_WwiseResourceLoaderSharesets);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadSharesetResources(const FWwiseSharesetCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading Shareset %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SharesetId);

	TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
	TArray<const FWwiseMediaCookedData*> LoadedMedia;
	bool bResult = true;

	if (LIKELY(bResult)) for (const auto& Media : InCookedData.Media)
	{
		if (UNLIKELY(!LoadMediaResources(Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSharesetResources: Could not load Media %s (%" PRIu32 ") for Shareset %s (%" PRIu32 ")"),
				*Media.DebugName, (uint32)Media.MediaId,
				*InCookedData.DebugName, (uint32)InCookedData.SharesetId);

			bResult = false;
			break;
		}
		LoadedMedia.Add(&Media);
	}

	if (LIKELY(bResult)) for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		if (UNLIKELY(!LoadSoundBankResources(SoundBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSharesetResources: Could not load SoundBank %s (%" PRIu32 ") for Shareset %s (%" PRIu32 ")"),
				*SoundBank.DebugName, (uint32)SoundBank.SoundBankId,
				*InCookedData.DebugName, (uint32)InCookedData.SharesetId);

			bResult = false;
			break;
		}
		LoadedSoundBanks.Add(&SoundBank);
	}

	if (UNLIKELY(!bResult))
	{
		for (const auto* SoundBank : LoadedSoundBanks)
		{
			UnloadSoundBankResources(*SoundBank);
		}
		for (const auto& Media : LoadedMedia)
		{
			UnloadMediaResources(*Media);
		}
	}
	return bResult;
}

void UWwiseResourceLoaderImpl::UnloadShareset(FWwiseLoadedSharesetListNode* InSharesetListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedShareset = InSharesetListNode->GetValue();
	const auto* Shareset = LoadedShareset.LocalizedSharesetCookedData.SharesetLanguageMap.Find(LoadedShareset.LanguageRef);
	if (UNLIKELY(!Shareset))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareset: Could not find Shareset %s in language %" PRIu32),
			*LoadedShareset.LocalizedSharesetCookedData.DebugName, (uint32)LoadedShareset.LanguageRef.GetLanguageId());
		return;
	}

	UnloadSharesetResources(*Shareset);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedSharesetList: %d] Shareset %s in language %" PRIu32),
		LoadedSharesetList.Num() - 1, *LoadedShareset.LocalizedSharesetCookedData.DebugName, (uint32)LoadedShareset.LanguageRef.GetLanguageId());
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSharesets);
	LoadedSharesetList.RemoveNode(InSharesetListNode);
}

void UWwiseResourceLoaderImpl::UnloadSharesetResources(const FWwiseSharesetCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading Shareset %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SharesetId);

	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		UnloadSoundBankResources(SoundBank);
	}
	for (const auto& Media : InCookedData.Media)
	{
		UnloadMediaResources(Media);
	}
}

//
// SoundBank resource handling
//

FWwiseLoadedSoundBankListNode* UWwiseResourceLoaderImpl::LoadSoundBank(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading SoundBank %s"), *InSoundBankCookedData.DebugName);

	const auto* LanguageKey = GetLanguageMapKey(InSoundBankCookedData.SoundBankLanguageMap, InLanguageOverride, InSoundBankCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBank: Could not find language for SoundBank %s"), *InSoundBankCookedData.DebugName);
		return nullptr;
	}

	const auto* SoundBank = InSoundBankCookedData.SoundBankLanguageMap.Find(*LanguageKey);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBank: Could not find SoundBank %s in language %" PRIu32),
			*InSoundBankCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	if (UNLIKELY(!LoadSoundBankResources(*SoundBank)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBank: Could not load SoundBank %s in language %" PRIu32),
			*InSoundBankCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
		return nullptr;
	}

	auto LoadedSoundBank = FWwiseLoadedSoundBank(InSoundBankCookedData, *LanguageKey);
	auto* LoadedNode = new FWwiseLoadedSoundBankListNode(LoadedSoundBank);
	LoadedSoundBankList.AddTail(LoadedNode);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedSoundBankList: %d] SoundBank %s in language %" PRIu32),
		LoadedSoundBankList.Num(), *InSoundBankCookedData.DebugName, (uint32)LanguageKey->GetLanguageId());
	INC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
	return LoadedNode;
}

bool UWwiseResourceLoaderImpl::LoadSoundBankResources(const FWwiseSoundBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);

	if (UNLIKELY(!DefaultLoadSoundBankOperation(InCookedData)))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankResources: Could not load SoundBank %s (%" PRIu32 ")"),
			*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);
		return false;
	}

	return true;
}

void UWwiseResourceLoaderImpl::UnloadSoundBank(FWwiseLoadedSoundBankListNode* InSoundBankListNode)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);
	const auto& LoadedSoundBank = InSoundBankListNode->GetValue();
	const auto* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadSoundBank: Could not find SoundBank %s in language %" PRIu32),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, (uint32)LoadedSoundBank.LanguageRef.GetLanguageId());
		return;
	}

	UnloadSoundBankResources(*SoundBank);

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadedSoundBankList: %d] SoundBank %s in language %" PRIu32),
		LoadedSoundBankList.Num() - 1, *LoadedSoundBank.LocalizedSoundBankCookedData.DebugName, (uint32)LoadedSoundBank.LanguageRef.GetLanguageId());
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
	LoadedSoundBankList.RemoveNode(InSoundBankListNode);
}

void UWwiseResourceLoaderImpl::UnloadSoundBankResources(const FWwiseSoundBankCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading SoundBank %s (%" PRIu32 ") resources"),
		*InCookedData.DebugName, (uint32)InCookedData.SoundBankId);

	DefaultUnloadSoundBankOperation(InCookedData);
}

//
// Switch Container resource handling
//

bool UWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources(const FWwiseSwitchContainerLeafCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading Switch Container"));

	TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
	TArray<const FWwiseMediaCookedData*> LoadedMedia;
	TArray<const FWwiseExternalSourceCookedData*> LoadedExternalSources;
	bool bResult = true;

	if (LIKELY(bResult)) for (const auto& Media : InCookedData.Media)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- Loading Media %s (%" PRIu32 ")"), *Media.DebugName, (uint32)Media.MediaId);
		if (UNLIKELY(!LoadMediaResources(Media)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSwitchContainerLeafResources: Could not load Media %s (%" PRIu32 ") for Switch Container"),
				*Media.DebugName, (uint32)Media.MediaId);

			bResult = false;
			break;
		}
		LoadedMedia.Add(&Media);
	}

	if (LIKELY(bResult)) for (const auto& ExternalSource : InCookedData.ExternalSources)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- Loading External Source %s (%" PRIu32 ")"), *ExternalSource.DebugName, (uint32)ExternalSource.Cookie);
		if (UNLIKELY(!LoadExternalSourceResources(ExternalSource)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSwitchContainerLeafResources: Could not load External Source %s (%" PRIu32 ") for Switch Container"),
				*ExternalSource.DebugName, (uint32)ExternalSource.Cookie);

			bResult = false;
			break;
		}
		LoadedExternalSources.Add(&ExternalSource);
	}

	if (LIKELY(bResult)) for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("- Loading SoundBank %s (%" PRIu32 ")"), *SoundBank.DebugName, (uint32)SoundBank.SoundBankId);
		if (UNLIKELY(!LoadSoundBankResources(SoundBank)))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSwitchContainerLeafResources: Could not load SoundBank %s (%" PRIu32 ") for Switch Container"),
				*SoundBank.DebugName, (uint32)SoundBank.SoundBankId);

			bResult = false;
			break;
		}
		LoadedSoundBanks.Add(&SoundBank);
	}

	if (LIKELY(bResult))
	{
		INC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);
	}
	else
	{
		for (const auto* SoundBank : LoadedSoundBanks)
		{
			UnloadSoundBankResources(*SoundBank);
		}
		for (const auto* Media : LoadedMedia)
		{
			UnloadMediaResources(*Media);
		}
		for (const auto* ExternalSource : LoadedExternalSources)
		{
			UnloadExternalSourceResources(*ExternalSource);
		}
	}

	return bResult;
}

bool UWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources(const FWwiseSwitchContainerLeafCookedData& InCookedData)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading Switch Container media"));

	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		UnloadSoundBankResources(SoundBank);
	}

	for (const auto& Media : InCookedData.Media)
	{
		UnloadMediaResources(Media);
	}

	for (const auto& ExternalSource : InCookedData.ExternalSources)
	{
		UnloadExternalSourceResources(ExternalSource);
	}

	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);
	return true;
}
