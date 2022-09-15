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

#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataSwitch.h"

const TCHAR* const FWwiseRefSwitch::NAME = TEXT("Switch");

const FWwiseMetadataSwitch* FWwiseRefSwitch::GetSwitch() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return nullptr;
	}
	const auto& Switches = SwitchGroup->Switches;
	if (Switches.IsValidIndex(SwitchIndex))
	{
		return &Switches[SwitchIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Switch index #%zu"), SwitchIndex);
		return nullptr;
	}
}

uint32 FWwiseRefSwitch::SwitchId() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return 0;
	}
	return Switch->Id;
}

FGuid FWwiseRefSwitch::SwitchGuid() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->GUID;
}

FString FWwiseRefSwitch::SwitchName() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->Name;
}

FString FWwiseRefSwitch::SwitchObjectPath() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->ObjectPath;
}

uint32 FWwiseRefSwitch::Hash() const
{
	auto Result = FWwiseRefSwitchGroup::Hash();
	Result = HashCombine(Result, ::GetTypeHash(SwitchIndex));
	return Result;
}
