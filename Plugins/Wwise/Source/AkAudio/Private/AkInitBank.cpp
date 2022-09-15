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

#include "AkInitBank.h"

#include "Platforms/AkPlatformInfo.h"
#include "Wwise/WwiseResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

#if WITH_EDITORONLY_DATA
void UAkInitBank::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
                                              TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile)
{
	auto* ResourceCooker = UWwiseResourceCooker::GetForPlatform(TargetPlatform);
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	ResourceCooker->SetSandboxRootPath(PackageFilename);
	ResourceCooker->CookInitBank(FWwiseAssetInfo::DefaultInitBank, WriteAdditionalFile);
}

void UAkInitBank::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	auto PlatformID = UAkPlatformInfo::GetSharedPlatformInfo(TargetPlatform->PlatformName());
	UWwiseResourceCooker::CreateForPlatform(TargetPlatform, PlatformID, EWwiseExportDebugNameRule::Name);
}
#endif

void UAkInitBank::BeginDestroy()
{
	Super::BeginDestroy();
	UnloadInitBank();
}

void UAkInitBank::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

#if WITH_EDITORONLY_DATA
	auto* ResourceCooker = UWwiseResourceCooker::GetForArchive(Ar);
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	FWwiseInitBankCookedData CookedDataToArchive;
	if (ResourceCooker->PrepareCookedData(CookedDataToArchive, FWwiseAssetInfo::DefaultInitBank))
	{
		CookedDataToArchive.Serialize(Ar);
	}
#else
	InitBankCookedData.Serialize(Ar);
#endif
}

void UAkInitBank::UnloadInitBank()
{
	if (LoadedInitBank )
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}

		ResourceLoader->UnloadInitBank(LoadedInitBank);
		LoadedInitBank=nullptr;
	}
}

#if WITH_EDITORONLY_DATA
void UAkInitBank::PrepareCookedData()
{
	if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
	{
		return;
	}
	auto* ResourceCooker = UWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(InitBankCookedData, FWwiseAssetInfo::DefaultInitBank)))
	{
		return;
	}
}
#endif

TArray<FWwiseLanguageCookedData> UAkInitBank::GetLanguages()
{
#if WITH_EDITORONLY_DATA
	PrepareCookedData();
#endif

	return InitBankCookedData.Language;
}


void UAkInitBank::LoadInitBank(bool bReload)
{
	auto* ResourceLoader = UWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	if (bReload)
	{
		UnloadInitBank();
	}
#if WITH_EDITORONLY_DATA
	PrepareCookedData();
#endif
	LoadedInitBank = ResourceLoader->LoadInitBank(InitBankCookedData);
}


#if WITH_EDITORONLY_DATA
void UAkInitBank::MigrateIds()
{
	//Do nothing because the DefaultInitBank info is used
}

FWwiseBasicInfo* UAkInitBank::GetInfoMutable()
{
	return new FWwiseBasicInfo(FWwiseAssetInfo::DefaultInitBank.AssetGuid, FWwiseAssetInfo::DefaultInitBank.AssetShortId, FWwiseAssetInfo::DefaultInitBank.AssetName);
}
#endif