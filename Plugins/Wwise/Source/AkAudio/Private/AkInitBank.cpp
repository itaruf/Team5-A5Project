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
	auto* ResourceCooker = FWwiseResourceCooker::GetForPlatform(TargetPlatform);
	if (!ResourceCooker)
	{
		return;
	}
	ResourceCooker->SetSandboxRootPath(PackageFilename);
	ResourceCooker->CookInitBank(FWwiseObjectInfo::DefaultInitBank, WriteAdditionalFile);
}

void UAkInitBank::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	auto PlatformID = UAkPlatformInfo::GetSharedPlatformInfo(TargetPlatform->IniPlatformName());
	FWwiseResourceCooker::CreateForPlatform(TargetPlatform, PlatformID, EWwiseExportDebugNameRule::Name);
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

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

 #if !UE_SERVER
 #if WITH_EDITORONLY_DATA
 	if (Ar.IsCooking() && Ar.IsSaving())
	{
		FWwiseInitBankCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, FWwiseObjectInfo::DefaultInitBank);
		}
		CookedDataToArchive.Serialize(Ar);
	}
 #else
 	InitBankCookedData.Serialize(Ar);
 #endif
 #endif
}

void UAkInitBank::UnloadInitBank()
{
	if (LoadedInitBank)
	{
		auto* ResourceLoader = FWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}

		ResourceLoader->UnloadInitBank(MoveTemp(LoadedInitBank));
		LoadedInitBank = nullptr;
	}
}

#if WITH_EDITORONLY_DATA
void UAkInitBank::PrepareCookedData()
{
	if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
	{
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(InitBankCookedData, FWwiseObjectInfo::DefaultInitBank)))
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


void UAkInitBank::LoadInitBank()
{
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	if (LoadedInitBank)
	{
		UnloadInitBank();
	}
#if WITH_EDITORONLY_DATA
	PrepareCookedData();
#endif
	LoadedInitBank = ResourceLoader->LoadInitBank(InitBankCookedData);
}


#if WITH_EDITORONLY_DATA
void UAkInitBank::MigrateWwiseObjectInfo()
{
	//Do nothing because the DefaultInitBank info is used
}

FWwiseObjectInfo* UAkInitBank::GetInfoMutable()
{
	return new FWwiseObjectInfo(FWwiseObjectInfo::DefaultInitBank.WwiseGuid, FWwiseObjectInfo::DefaultInitBank.WwiseShortId, FWwiseObjectInfo::DefaultInitBank.WwiseName);
}
#endif