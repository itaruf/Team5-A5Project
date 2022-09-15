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

#include "Wwise/Ref/WwiseRefDialogueArgument.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataDialogue.h"

const TCHAR* const FWwiseRefDialogueArgument::NAME = TEXT("DialogueArgument");

const FWwiseMetadataDialogueArgument* FWwiseRefDialogueArgument::GetDialogueArgument() const
{
	const auto* DialogueEvent = GetDialogueEvent();
	if (UNLIKELY(!DialogueEvent))
	{
		return nullptr;
	}
	const auto& Arguments = DialogueEvent->Arguments;
	if (Arguments.IsValidIndex(DialogueArgumentIndex))
	{
		return &Arguments[DialogueArgumentIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Dialogue Argument index #%zu"), DialogueArgumentIndex);
		return nullptr;
	}
}

uint32 FWwiseRefDialogueArgument::DialogueArgumentId() const
{
	const auto* DialogueArgument = GetDialogueArgument();
	if (UNLIKELY(!DialogueArgument))
	{
		return 0;
	}
	return DialogueArgument->Id;
}

FGuid FWwiseRefDialogueArgument::DialogueArgumentGuid() const
{
	const auto* DialogueArgument = GetDialogueArgument();
	if (UNLIKELY(!DialogueArgument))
	{
		return {};
	}
	return DialogueArgument->GUID;
}

FString FWwiseRefDialogueArgument::DialogueArgumentName() const
{
	const auto* DialogueArgument = GetDialogueArgument();
	if (UNLIKELY(!DialogueArgument))
	{
		return {};
	}
	return DialogueArgument->Name;
}

FString FWwiseRefDialogueArgument::DialogueArgumentObjectPath() const
{
	const auto* DialogueArgument = GetDialogueArgument();
	if (UNLIKELY(!DialogueArgument))
	{
		return {};
	}
	return DialogueArgument->ObjectPath;
}

uint32 FWwiseRefDialogueArgument::Hash() const
{
	auto Result = FWwiseRefDialogueEvent::Hash();
	Result = HashCombine(Result, ::GetTypeHash(DialogueArgumentIndex));
	return Result;
}
