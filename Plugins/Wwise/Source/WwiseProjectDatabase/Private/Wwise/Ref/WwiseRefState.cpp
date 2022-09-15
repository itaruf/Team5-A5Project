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

#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataState.h"

const TCHAR* const FWwiseRefState::NAME = TEXT("State");

const FWwiseMetadataState* FWwiseRefState::GetState() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return nullptr;
	}
	const auto& States = StateGroup->States;
	if (States.IsValidIndex(StateIndex))
	{
		return &States[StateIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get State index #%zu"), StateIndex);
		return nullptr;
	}
}

uint32 FWwiseRefState::StateId() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return 0;
	}
	return State->Id;
}

FGuid FWwiseRefState::StateGuid() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->GUID;
}

FString FWwiseRefState::StateName() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->Name;
}

FString FWwiseRefState::StateObjectPath() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->ObjectPath;
}

uint32 FWwiseRefState::Hash() const
{
	auto Result = FWwiseRefStateGroup::Hash();
	Result = HashCombine(Result, ::GetTypeHash(StateIndex));
	return Result;
}
