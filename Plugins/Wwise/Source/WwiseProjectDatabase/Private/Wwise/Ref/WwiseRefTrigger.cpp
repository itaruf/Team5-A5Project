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

#include "Wwise/Ref/WwiseRefTrigger.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/FileHandler.h"
#include "Wwise/Metadata/WwiseMetadataTrigger.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefTrigger::NAME = TEXT("Trigger");

const FWwiseMetadataTrigger* FWwiseRefTrigger::GetTrigger() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& Triggers = SoundBank->Triggers;
	if (Triggers.IsValidIndex(TriggerIndex))
	{
		return &Triggers[TriggerIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Trigger index #%zu"), TriggerIndex);
		return nullptr;
	}
}

uint32 FWwiseRefTrigger::TriggerId() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return 0;
	}
	return Trigger->Id;
}

FGuid FWwiseRefTrigger::TriggerGuid() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->GUID;
}

FString FWwiseRefTrigger::TriggerName() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->Name;
}

FString FWwiseRefTrigger::TriggerObjectPath() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->ObjectPath;
}

uint32 FWwiseRefTrigger::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(TriggerIndex));
	return Result;
}
