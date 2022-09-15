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

#include "Wwise/Ref/WwiseRefSwitchGroup.h"

class WWISEPROJECTDATABASE_API FWwiseRefSwitch : public FWwiseRefSwitchGroup
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Switch;
	struct FGlobalIdsMap;

	WwiseRefIndexType SwitchIndex;

	FWwiseRefSwitch() {}
	FWwiseRefSwitch(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InSwitchGroupIndex,
		WwiseRefIndexType InSwitchIndex) :
		FWwiseRefSwitchGroup(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId, InSwitchGroupIndex),
		SwitchIndex(InSwitchIndex)
	{}
	const FWwiseMetadataSwitch* GetSwitch() const;

	uint32 SwitchId() const;
	FGuid SwitchGuid() const;
	FString SwitchName() const;
	FString SwitchObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSwitch& Rhs) const
	{
		return FWwiseRefSwitchGroup::operator ==(Rhs)
			&& SwitchIndex == Rhs.SwitchIndex;
	}
	bool operator!=(const FWwiseRefSwitch& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefSwitch::FGlobalIdsMap
{
	WwiseSwitchGlobalIdsMap GlobalIdsMap;
};
