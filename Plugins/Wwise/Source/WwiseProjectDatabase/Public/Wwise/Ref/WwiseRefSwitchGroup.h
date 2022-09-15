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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefSwitchGroup : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::SwitchGroup;
	struct FGlobalIdsMap;

	WwiseRefIndexType SwitchGroupIndex;

	FWwiseRefSwitchGroup() {}
	FWwiseRefSwitchGroup(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InSwitchGroupIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		SwitchGroupIndex(InSwitchGroupIndex)
	{}
	const FWwiseMetadataSwitchGroup* GetSwitchGroup() const;
	bool IsControlledByGameParameter() const;

	uint32 SwitchGroupId() const;
	FGuid SwitchGroupGuid() const;
	FString SwitchGroupName() const;
	FString SwitchGroupObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSwitchGroup& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& SwitchGroupIndex == Rhs.SwitchGroupIndex;
	}
	bool operator!=(const FWwiseRefSwitchGroup& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefSwitchGroup::FGlobalIdsMap
{
	WwiseSwitchGroupGlobalIdsMap GlobalIdsMap;
};
