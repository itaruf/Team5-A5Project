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

#include "Wwise/Ref/WwiseRefDialogueEvent.h"

#include "Wwise/Ref/WwiseRefCollections.h"
#include "Wwise/Ref/WwiseRefDialogueArgument.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Metadata/WwiseMetadataDialogue.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

#include <inttypes.h>


const TCHAR* const FWwiseRefDialogueEvent::NAME = TEXT("DialogueEvent");

const FWwiseMetadataDialogueEvent* FWwiseRefDialogueEvent::GetDialogueEvent() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& DialogueEvents = SoundBank->DialogueEvents;
	if (DialogueEvents.IsValidIndex(DialogueEventIndex))
	{
		return &DialogueEvents[DialogueEventIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Dialogue Event index #%zu"), DialogueEventIndex);
		return nullptr;
	}
}

WwiseDialogueArgumentIdsMap FWwiseRefDialogueEvent::GetDialogueArguments(const WwiseDialogueArgumentGlobalIdsMap& GlobalMap) const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (!DialogueEvent)
	{
		return {};
	}
	const auto Arguments = DialogueEvent->Arguments;
	WwiseDialogueArgumentIdsMap Result;
	Result.Empty(Arguments.Num());
	for (const auto& Elem : Arguments)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Dialogue Argument ID %" PRIu32), Elem.Id);
		}
	}

	return Result;
}

uint32 FWwiseRefDialogueEvent::DialogueEventId() const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (UNLIKELY(!DialogueEvent))
	{
		return 0;
	}
	return DialogueEvent->Id;
}

FGuid FWwiseRefDialogueEvent::DialogueEventGuid() const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (UNLIKELY(!DialogueEvent))
	{
		return {};
	}
	return DialogueEvent->GUID;
}

FString FWwiseRefDialogueEvent::DialogueEventName() const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (UNLIKELY(!DialogueEvent))
	{
		return {};
	}
	return DialogueEvent->Name;
}

FString FWwiseRefDialogueEvent::DialogueEventObjectPath() const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (UNLIKELY(!DialogueEvent))
	{
		return {};
	}
	return DialogueEvent->ObjectPath;
}

uint32 FWwiseRefDialogueEvent::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(DialogueEventIndex));
	return Result;
}
