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

#include "AkEffectShareset.h"

#include "Wwise/WwiseResourceLoader.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkEffectShareSet::Serialize(FArchive& Ar)
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

	FWwiseLocalizedSharesetCookedData CookedDataToArchive;
	if (ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(SharesetInfo)))
	{
		CookedDataToArchive.Serialize(Ar);
	}

#else
	SharesetCookedData.Serialize(Ar);
#endif

}

void UAkEffectShareSet::LoadEffectShareset(bool bReload)
{
	auto* ResourceLoader = UWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	
	if (bReload)
	{
		UnloadEffectShareset();
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
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(SharesetCookedData, GetValidatedInfo(SharesetInfo))))
	{
		return;
	}
#endif

	LoadedShareset = ResourceLoader->LoadShareset(SharesetCookedData);
}

void UAkEffectShareSet::UnloadEffectShareset()
{
	if (LoadedShareset)
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		ResourceLoader->UnloadShareset(LoadedShareset);
		LoadedShareset=nullptr;
	}
}

void UAkEffectShareSet::PostLoad()
{
	Super::PostLoad();
	LoadEffectShareset(false);
}

void UAkEffectShareSet::BeginDestroy()
{
	Super::BeginDestroy();
	UnloadEffectShareset();
}

#if WITH_EDITOR
void UAkEffectShareSet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	LoadEffectShareset(true);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITORONLY_DATA
void UAkEffectShareSet::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
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
	ResourceCooker->CookShareset(GetValidatedInfo(SharesetInfo), WriteAdditionalFile);
}
#endif