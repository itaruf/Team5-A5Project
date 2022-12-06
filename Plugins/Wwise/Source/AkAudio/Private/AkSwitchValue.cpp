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

#include "AkSwitchValue.h"

#include "Wwise/WwiseResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#include "AkAudioDevice.h"
#endif

void UAkSwitchValue::LoadGroupValue()
{
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	
	if (LoadedGroupValue)
	{
		UnloadGroupValue();
	}

#if WITH_EDITORONLY_DATA
	if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkSwitchValue::LoadGroupValue: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(GroupValueCookedData, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::Switch)))
	{
		return;
	}
#endif
	LoadedGroupValue = ResourceLoader->LoadGroupValue(GroupValueCookedData);
}

void UAkSwitchValue::Serialize(FArchive& Ar)
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
		FWwiseGroupValueCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::Switch);
		}
		CookedDataToArchive.Serialize(Ar);
	}
#else
	GroupValueCookedData.Serialize(Ar);
#endif
#endif
}

#if WITH_EDITORONLY_DATA
void UAkSwitchValue::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	FWwiseObjectInfo* AudioTypeInfo = GetInfoMutable();
	const FWwiseObjectInfo WwiseObjectInfo = FWwiseObjectInfo(AudioTypeInfo->WwiseGuid, AudioTypeInfo->WwiseShortId, AudioTypeInfo->WwiseName);
	FWwiseRefSwitch RefSwitch = FWwiseDataStructureScopeLock(*ProjectDatabase).GetSwitch(
		GetValidatedInfo(GroupValueInfo));

	if (RefSwitch.SwitchName().IsNone() || !RefSwitch.SwitchGuid().IsValid() || RefSwitch.SwitchId() == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkSwitchValue::FillInfo: Valid object not found in Project Database"));
		return;
	}

	AudioTypeInfo->WwiseName = RefSwitch.SwitchName();
	AudioTypeInfo->WwiseGuid = RefSwitch.SwitchGuid();
	AudioTypeInfo->WwiseShortId = RefSwitch.SwitchId();
}
#endif

