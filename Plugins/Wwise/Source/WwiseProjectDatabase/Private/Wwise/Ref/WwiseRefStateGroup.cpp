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

#include "Wwise/Ref/WwiseRefStateGroup.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefStateGroup::NAME = TEXT("StateGroup");

const FWwiseMetadataStateGroup* FWwiseRefStateGroup::GetStateGroup() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& StateGroups = SoundBank->StateGroups;
	if (StateGroups.IsValidIndex(StateGroupIndex))
	{
		return &StateGroups[StateGroupIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get State Group index #%zu"), StateGroupIndex);
		return nullptr;
	}
}

uint32 FWwiseRefStateGroup::StateGroupId() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return 0;
	}
	return StateGroup->Id;
}

FGuid FWwiseRefStateGroup::StateGroupGuid() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->GUID;
}

FString FWwiseRefStateGroup::StateGroupName() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->Name;
}

FString FWwiseRefStateGroup::StateGroupObjectPath() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->ObjectPath;
}

uint32 FWwiseRefStateGroup::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(StateGroupIndex));
	return Result;
}
