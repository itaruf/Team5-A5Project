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

#pragma once

#include "Wwise/Ref/WwiseRefStateGroup.h"

class WWISEPROJECTDATABASE_API FWwiseRefState : public FWwiseRefStateGroup
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::State;
	struct FGlobalIdsMap;

	WwiseRefIndexType StateIndex;

	FWwiseRefState() {}
	FWwiseRefState(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InStateGroupIndex,
		WwiseRefIndexType InStateIndex) :
		FWwiseRefStateGroup(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId, InStateGroupIndex),
		StateIndex(InStateIndex)
	{}
	const FWwiseMetadataState* GetState() const;

	uint32 StateId() const;
	FGuid StateGuid() const;
	FString StateName() const;
	FString StateObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefState& Rhs) const
	{
		return FWwiseRefStateGroup::operator ==(Rhs)
			&& StateIndex == Rhs.StateIndex;
	}
	bool operator!=(const FWwiseRefState& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefState::FGlobalIdsMap
{
	WwiseStateGlobalIdsMap GlobalIdsMap;
};
