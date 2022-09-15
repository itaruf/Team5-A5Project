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

#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include <inttypes.h>

const TCHAR* const FWwiseRefAuxBus::NAME = TEXT("AuxBus");

const FWwiseMetadataBus* FWwiseRefAuxBus::GetAuxBus() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& AuxBusses = SoundBank->AuxBusses;
	if (AuxBusses.IsValidIndex(AuxBusIndex))
	{
		return &AuxBusses[AuxBusIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get AuxBus index #%zu"), AuxBusIndex);
		return nullptr;
	}
}

void FWwiseRefAuxBus::GetAllAuxBusRefs(TSet<const FWwiseRefAuxBus*>& OutAuxBusRefs, const WwiseAuxBusGlobalIdsMap& InGlobalMap) const
{
	bool bIsAlreadyInSet = false;
	OutAuxBusRefs.Add(this, &bIsAlreadyInSet);
	if (UNLIKELY(bIsAlreadyInSet))		// Unlikely but can still be done (circular references are possible in Aux Busses)
	{
		return;
	}

	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return;
	}
	for (const auto& SubAuxBus : AuxBus->AuxBusRefs)
	{
		const auto* SubAuxBusRef = InGlobalMap.Find(FWwiseDatabaseLocalizableIdKey(SubAuxBus.Id, LanguageId));
		if (UNLIKELY(!SubAuxBusRef))
		{
			SubAuxBusRef = InGlobalMap.Find(FWwiseDatabaseLocalizableIdKey(SubAuxBus.Id, 0));
		}
		if (UNLIKELY(!SubAuxBusRef))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Aux Bus Id %" PRIu32), SubAuxBus.Id);
			continue;
		}
		SubAuxBusRef->GetAllAuxBusRefs(OutAuxBusRefs, InGlobalMap);
	}
}


uint32 FWwiseRefAuxBus::AuxBusId() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return 0;
	}
	return AuxBus->Id;
}

FGuid FWwiseRefAuxBus::AuxBusGuid() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->GUID;
}

FString FWwiseRefAuxBus::AuxBusName() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->Name;
}

FString FWwiseRefAuxBus::AuxBusObjectPath() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->ObjectPath;
}

uint32 FWwiseRefAuxBus::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(AuxBusIndex));
	return Result;
}
