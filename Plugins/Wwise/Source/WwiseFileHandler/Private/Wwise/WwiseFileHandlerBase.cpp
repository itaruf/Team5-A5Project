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

#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

#include "Wwise/Stats/FileHandler.h"
#include "Wwise/Stats/AsyncStats.h"

#include <inttypes.h>


FWwiseFileHandlerBase::FWwiseFileHandlerBase()
{
}

AKRESULT FWwiseFileHandlerBase::OpenStreaming(AkFileDesc& OutFileDesc, uint32 InShortId)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));

	AKRESULT Result = AK_Success;
	FEventRef WaitForCallback(EEventMode::ManualReset);
	IncrementFileStateUseAsync(InShortId, EWwiseFileStateOperationOrigin::Streaming,
		[ManagingTypeName = GetManagingTypeName(), InShortId]
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Trying to open streaming for unknown %s %" PRIu32), ManagingTypeName, InShortId);
			return FWwiseFileStateSharedPtr{};
		},
		[&OutFileDesc, InShortId, &Result, &WaitForCallback](const FWwiseFileStateSharedPtr& InFileState, bool bInResult) mutable
		{
			if (UNLIKELY(!InFileState.IsValid() || !bInResult))
			{
				Result = AK_FileNotFound;
			}
			else if (const auto* StreamableFileStateInfo = InFileState->GetStreamableFileStateInfo())
			{
				StreamableFileStateInfo->GetFileDescCopy(OutFileDesc);
			}
			else
			{
				Result = AK_UnknownFileError;
			}
			WaitForCallback->Trigger();
		});
	WaitForCallback->Wait();

	return Result;
}

void FWwiseFileHandlerBase::CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState)
{
	return DecrementFileStateUseAsync(InShortId, &InFileState, EWwiseFileStateOperationOrigin::Streaming, []{});
}

void FWwiseFileHandlerBase::IncrementFileStateUseAsync(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async([this, InShortId, InOperationOrigin, InCreate = MoveTemp(InCreate), InCallback = MoveTemp(InCallback)]() mutable
	{
		IncrementFileStateUse(InShortId, InOperationOrigin, MoveTemp(InCreate), MoveTemp(InCallback));
	});
}

void FWwiseFileHandlerBase::DecrementFileStateUseAsync(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async([this, InShortId, InFileState, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
	{
		DecrementFileStateUse(InShortId, InFileState, InOperationOrigin, MoveTemp(InCallback));
	});
}

void FWwiseFileHandlerBase::IncrementFileStateUse(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback)
{
	FWwiseFileStateSharedPtr State;
	if (const auto* StatePtr = FileStatesById.Find(InShortId))
	{
		State = *StatePtr;
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Creating new State for %s %" PRIu32), GetManagingTypeName(), InShortId);
		State = InCreate();
		if (LIKELY(State.IsValid()))
		{
			FileStatesById.Add(InShortId, State);
		}
	}

	if (UNLIKELY(!State.IsValid()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Trying to increment invalid state for %s %" PRIu32), GetManagingTypeName(), InShortId);
		InCallback(State, false);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Incrementing State for %s %" PRIu32), GetManagingTypeName(), InShortId);
		State->IncrementCountAsync(InOperationOrigin, [State, InCallback = MoveTemp(InCallback)](bool bInResult)
		{
			InCallback(State, bInResult);
		});
	}
}

void FWwiseFileHandlerBase::DecrementFileStateUse(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	if (!InFileState)
	{
		const auto* StatePtr = FileStatesById.Find(InShortId);
		if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("Could not find state for for %s %" PRIu32), GetManagingTypeName(), InShortId);
			InCallback();
			return;
		}
		InFileState = StatePtr->Get();
	}

	InFileState->DecrementCountAsync(InOperationOrigin, [this, InShortId, InFileState, InOperationOrigin]() mutable
	{
		// File state deletion request
		FileHandlerExecutionQueue.Async([this, InShortId, InFileState, InOperationOrigin]() mutable
		{
			OnDeleteState(InShortId, *InFileState, InOperationOrigin);
		});
	}, MoveTemp(InCallback));
}

void FWwiseFileHandlerBase::OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin)
{
	if (!InFileState.CanDelete())
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OnDeleteState %s %" PRIu32 ": Cannot delete State. Probably re-loaded between deletion request and now."),
			GetManagingTypeName(), InShortId);
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OnDeleteState %s %" PRIu32 ": Deleting."), GetManagingTypeName(), InShortId);
	const auto RemovalCount = FileStatesById.Remove(InShortId);		// WARNING: This will very probably delete InFileState reference. Do not use the File State from that point!

	UE_CLOG(RemovalCount != 1, LogWwiseFileHandler, Error, TEXT("Removing a state for %s %" PRIu32 ", ended up deleting %" PRIi32 " states."),
		GetManagingTypeName(), InShortId, RemovalCount);
}
