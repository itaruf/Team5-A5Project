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

#include "Wwise/Ref/WwiseRefSwitchGroup.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"

const TCHAR* const FWwiseRefSwitchGroup::NAME = TEXT("SwitchGroup");

const FWwiseMetadataSwitchGroup* FWwiseRefSwitchGroup::GetSwitchGroup() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& SwitchGroups = SoundBank->SwitchGroups;
	if (SwitchGroups.IsValidIndex(SwitchGroupIndex))
	{
		return &SwitchGroups[SwitchGroupIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Switch Group index #%zu"), SwitchGroupIndex);
		return nullptr;
	}
}

bool FWwiseRefSwitchGroup::IsControlledByGameParameter() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (!SwitchGroup)
	{
		return false;
	}

	return SwitchGroup->GameParameterRef != nullptr;
}

uint32 FWwiseRefSwitchGroup::SwitchGroupId() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return 0;
	}
	return SwitchGroup->Id;
}

FGuid FWwiseRefSwitchGroup::SwitchGroupGuid() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->GUID;
}

FString FWwiseRefSwitchGroup::SwitchGroupName() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->Name;
}

FString FWwiseRefSwitchGroup::SwitchGroupObjectPath() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->ObjectPath;
}

uint32 FWwiseRefSwitchGroup::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(SwitchGroupIndex));
	return Result;
}
