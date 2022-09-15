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

#include "Wwise/Ref/WwiseRefDialogueEvent.h"

class WWISEPROJECTDATABASE_API FWwiseRefDialogueArgument : public FWwiseRefDialogueEvent
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::DialogueArgument;
	struct FGlobalIdsMap;

	WwiseRefIndexType DialogueArgumentIndex;

	FWwiseRefDialogueArgument() {}
	FWwiseRefDialogueArgument(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InDialogueEventIndex,
		WwiseRefIndexType InDialogueArgumentIndex) :
		FWwiseRefDialogueEvent(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId, InDialogueEventIndex),
		DialogueArgumentIndex(InDialogueArgumentIndex)
	{}
	const FWwiseMetadataDialogueArgument* GetDialogueArgument() const;

	uint32 DialogueArgumentId() const;
	FGuid DialogueArgumentGuid() const;
	FString DialogueArgumentName() const;
	FString DialogueArgumentObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefDialogueArgument& Rhs) const
	{
		return FWwiseRefDialogueEvent::operator ==(Rhs)
			&& DialogueArgumentIndex == Rhs.DialogueArgumentIndex;
	}
	bool operator!=(const FWwiseRefDialogueArgument& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefDialogueArgument::FGlobalIdsMap
{
	WwiseDialogueArgumentGlobalIdsMap GlobalIdsMap;
};
