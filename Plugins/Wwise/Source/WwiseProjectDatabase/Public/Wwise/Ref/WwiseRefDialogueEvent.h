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

class WWISEPROJECTDATABASE_API FWwiseRefDialogueEvent : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::DialogueEvent;
	struct FGlobalIdsMap;

	WwiseRefIndexType DialogueEventIndex;

	FWwiseRefDialogueEvent() {}
	FWwiseRefDialogueEvent(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InDialogueEventIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		DialogueEventIndex(InDialogueEventIndex)
	{}
	const FWwiseMetadataDialogueEvent* GetDialogueEvent() const;
	WwiseDialogueArgumentIdsMap GetDialogueArguments(const WwiseDialogueArgumentGlobalIdsMap& GlobalMap) const;

	uint32 DialogueEventId() const;
	FGuid DialogueEventGuid() const;
	FString DialogueEventName() const;
	FString DialogueEventObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefDialogueEvent& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& DialogueEventIndex == Rhs.DialogueEventIndex;
	}
	bool operator!=(const FWwiseRefDialogueEvent& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefDialogueEvent::FGlobalIdsMap
{
	WwiseDialogueEventGlobalIdsMap GlobalIdsMap;
};
