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
	AkEvent.cpp:
=============================================================================*/

#include "AkAudioEvent.h"
#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "Wwise/WwiseResourceLoader.h"


#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkAudioEvent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	auto* ResourceCooker = UWwiseResourceCooker::GetForArchive(Ar);
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}

	if (ResourceCooker->PrepareCookedData(EventCookedData, GetValidatedInfo(EventInfo)))
	{
		EventCookedData.Serialize(Ar);
	}

	FillMetadata(ResourceCooker->GetProjectDatabase());

	if (Ar.IsCooking() && Ar.IsSaving())
	{
		Ar << MaximumDuration;
		Ar << MinimumDuration;
		Ar << IsInfinite;
		Ar << MaxAttenuationRadius;
	}

#else
	EventCookedData.Serialize(Ar);
	Ar << MaximumDuration;
	Ar << MinimumDuration;
	Ar << IsInfinite;
	Ar << MaxAttenuationRadius;
#endif
}

#if WITH_EDITORONLY_DATA
void UAkAudioEvent::FillMetadata(UWwiseProjectDatabase * ProjectDatabase)
{
	const TSet<FWwiseRefEvent> EventRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetEvent(GetValidatedInfo(EventInfo));
	if (UNLIKELY(EventRef.Num() == 0))
	{
		UE_LOG(LogAkAudio, Log, TEXT("AkAudioEvent (%s): Cannot fill Metadata - Event is not loaded"), *GetName());
		return;
	}
	const FWwiseMetadataEvent* EventMetadata = EventRef.Array()[0].GetEvent();
	MaximumDuration = EventMetadata->DurationMax;
	MinimumDuration = EventMetadata->DurationMin;
	IsInfinite = EventMetadata->DurationType == EWwiseMetadataEventDurationType::Infinite;
	MaxAttenuationRadius = EventMetadata->MaxAttenuation;
}
#endif


void UAkAudioEvent::LoadEventData(bool bReload)
{
	auto* ResourceLoader = UWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}

	if (bReload)
	{
		UnloadEventData();
	}
	
#if WITH_EDITORONLY_DATA
	if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
	{
		return;
	}
	auto* ResourceCooker = UWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(EventCookedData, GetValidatedInfo(EventInfo))))
	{
		return;
	}
	FillMetadata(ResourceCooker->GetProjectDatabase());
#endif

	UE_LOG(LogAkAudio, Verbose, TEXT("%s - LoadEventData"), *GetName());
	LoadedEventData = ResourceLoader->LoadEvent(EventCookedData);
}

void UAkAudioEvent::PostLoad()
{
	Super::PostLoad();
	LoadEventData(true);
}

void UAkAudioEvent::BeginDestroy()
{
	Super::BeginDestroy();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("%s - Event BeginDestroy"), *GetName());
	
	UnloadEventData();
}

void UAkAudioEvent::UnloadEventData()
{
	if (LoadedEventData)
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		UE_LOG(LogAkAudio, Verbose, TEXT("%s - UnloadEventData"), *GetName());
		ResourceLoader->UnloadEvent(LoadedEventData);
		LoadedEventData=nullptr;
	}
}

bool UAkAudioEvent::IsDataFullyLoaded() const
{
	if (!LoadedEventData)
	{
		return false;
	}

	return LoadedEventData->GetValue().bLoaded;
}

TArray<FWwiseExternalSourceCookedData> UAkAudioEvent::GetExternalSources() const
{
	auto* ResourceLoader = FWwiseResourceLoaderModule::GetModule()->GetResourceLoader();
	if (LIKELY(ResourceLoader))
	{
		const FWwiseResourceLoaderImplScopeLock ResourceLoaderImpl(ResourceLoader);
		auto CurrentLanguage = ResourceLoaderImpl->GetLanguageMapKey(EventCookedData.EventLanguageMap, nullptr, EventCookedData.DebugName );
		if (LIKELY(CurrentLanguage))
		{
			const FWwiseEventCookedData* CookedData = EventCookedData.EventLanguageMap.Find(*CurrentLanguage);
			if(LIKELY(CookedData))
			{
				return CookedData->ExternalSources;
			}
		}
	}

	return {};
}

#if WITH_EDITOR
void UAkAudioEvent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	LoadEventData(true);
}
#endif

#if WITH_EDITORONLY_DATA
void UAkAudioEvent::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
	TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	UWwiseResourceCooker* ResourceCooker = UWwiseResourceCooker::GetForPlatform(TargetPlatform);
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	ResourceCooker->SetSandboxRootPath(PackageFilename);
	ResourceCooker->CookEvent(GetValidatedInfo(EventInfo), WriteAdditionalFile);
}
#endif
