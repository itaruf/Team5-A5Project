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

#include "Wwise/Ref/WwiseRefBus.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefBus::NAME = TEXT("Bus");

const FWwiseMetadataBus* FWwiseRefBus::GetBus() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& Busses = SoundBank->Busses;
	if (Busses.IsValidIndex(BusIndex))
	{
		return &Busses[BusIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Bus index #%zu"), BusIndex);
		return nullptr;
	}
}

uint32 FWwiseRefBus::BusId() const
{
	const auto* Bus = GetBus();
	if (UNLIKELY(!Bus))
	{
		return 0;
	}
	return Bus->Id;
}

FGuid FWwiseRefBus::BusGuid() const
{
	const auto* Bus = GetBus();
	if (UNLIKELY(!Bus))
	{
		return {};
	}
	return Bus->GUID;
}

FString FWwiseRefBus::BusName() const
{
	const auto* Bus = GetBus();
	if (UNLIKELY(!Bus))
	{
		return {};
	}
	return Bus->Name;
}

FString FWwiseRefBus::BusObjectPath() const
{
	const auto* Bus = GetBus();
	if (UNLIKELY(!Bus))
	{
		return {};
	}
	return Bus->ObjectPath;
}

uint32 FWwiseRefBus::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(BusIndex));
	return Result;
}
