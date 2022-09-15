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

#include "AkStateValue.h"

#include "Wwise/WwiseResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkStateValue::LoadGroupValue(bool bReload)
{
	auto* ResourceLoader = UWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	
	if (bReload)
	{
		UnloadGroupValue();
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

	if (UNLIKELY(!ResourceCooker->PrepareCookedData(GroupValueCookedData, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::State)))
	{
		return;
	}
#endif
	LoadedGroupValue = ResourceLoader->LoadGroupValue(GroupValueCookedData);
}

void UAkStateValue::Serialize(FArchive& Ar)
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
	FWwiseGroupValueCookedData CookedDataToArchive;

	if (ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::State))
	{
		CookedDataToArchive.Serialize(Ar);
	}

#else
	GroupValueCookedData.Serialize(Ar);
#endif
}