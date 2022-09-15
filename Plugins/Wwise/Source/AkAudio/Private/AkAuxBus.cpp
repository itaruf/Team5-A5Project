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

#include "AkAuxBus.h"
#include "AkAudioBank.h"
#include "Wwise/WwiseResourceLoader.h"
#include "AkInclude.h"
#include "AkAudioDevice.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif


void UAkAuxBus::PostLoad()
{
	Super::PostLoad();

	LoadAuxBus(false);
}

void UAkAuxBus::Serialize(FArchive& Ar)
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

	FWwiseLocalizedAuxBusCookedData CookedDataToArchive;
	if (ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(AuxBusInfo)))
	{
		CookedDataToArchive.Serialize(Ar);
	}
#else
	AuxBusCookedData.Serialize(Ar);
#endif

}

void UAkAuxBus::LoadAuxBus(bool bReload)
{
	auto* ResourceLoader = UWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}

	if (bReload)
	{
		UnloadAuxBus();
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
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(AuxBusCookedData, GetValidatedInfo(AuxBusInfo))))
	{
		return;
	}
#endif

	LoadedAuxBusData = ResourceLoader->LoadAuxBus(AuxBusCookedData);
}

void UAkAuxBus::UnloadAuxBus()
{
	if (LoadedAuxBusData)
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		ResourceLoader->UnloadAuxBus(LoadedAuxBusData);
		LoadedAuxBusData=nullptr;
	}
}

void UAkAuxBus::BeginDestroy()
{
	Super::BeginDestroy();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("%s - UAkAuxBus BeginDestroy"), *GetName());

	UnloadAuxBus();
}

#if WITH_EDITOR
void UAkAuxBus::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	LoadAuxBus(true);
}
#endif

#if WITH_EDITORONLY_DATA
void UAkAuxBus::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
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

	ResourceCooker->CookAuxBus(GetValidatedInfo(AuxBusInfo), WriteAdditionalFile);
}
#endif