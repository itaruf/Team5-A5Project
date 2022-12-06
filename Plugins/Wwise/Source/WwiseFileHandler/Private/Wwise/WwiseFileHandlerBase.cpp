/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

#include "Wwise/Stats/FileHandler.h"
#include "Wwise/Stats/AsyncStats.h"

#include "Misc/ScopeRWLock.h"

#include <inttypes.h>


FWwiseFileHandlerBase::FWwiseFileHandlerBase()
{
}

AKRESULT FWwiseFileHandlerBase::OpenStreaming(AkFileDesc& OutFileDesc, uint32 InShortId)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));

	AKRESULT Result = AK_Success;

	if (UNLIKELY(OutFileDesc.pCustomParam != nullptr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: CustomParam is not empty"), InShortId);
		return AK_Fail;
	}
	auto* WaitForCallback = new FEventRef(EEventMode::ManualReset);
	OutFileDesc.pCustomParam = WaitForCallback;

	IncrementFileStateUseAsync(InShortId, EWwiseFileStateOperationOrigin::Streaming,
		[ManagingTypeName = GetManagingTypeName(), InShortId]
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Trying to open streaming for unknown %s %" PRIu32), ManagingTypeName, InShortId);
			return FWwiseFileStateSharedPtr{};
		},
		[InShortId, WaitForCallback, OpCycleCounter = MoveTemp(OpCycleCounter)](const FWwiseFileStateSharedPtr& InFileState, bool bInResult) mutable
		{
			OpCycleCounter.Stop();
			UE_CLOG(LIKELY(bInResult), LogWwiseFileHandler, VeryVerbose, TEXT("Succeeded opening %" PRIu32 " for streaming"), InShortId);
			UE_CLOG(UNLIKELY(!bInResult), LogWwiseFileHandler, VeryVerbose, TEXT("Failed opening %" PRIu32 " for streaming"), InShortId);
			(*WaitForCallback)->Trigger();
		});

	return Result;
}

AKRESULT FWwiseFileHandlerBase::GetOpenStreamingResult(AkFileDesc& OutFileDesc, uint32 InShortId)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));

	const auto* WaitForCallback = static_cast<FEventRef*>(OutFileDesc.pCustomParam);
	OutFileDesc.pCustomParam = nullptr;

	if (UNLIKELY(WaitForCallback == nullptr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: Open was not done prior to getting Results."), InShortId);
		return AK_Fail;
	}

	(*WaitForCallback)->Wait();
	delete WaitForCallback;

	FWwiseFileStateSharedPtr State;
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		const auto* StatePtr = FileStatesById.Find(InShortId);
		if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: File wasn't initialized prior to OpenStreaming."), InShortId);
			return AK_FileNotFound;
		}
		State = *StatePtr;
	}

	AKRESULT Result = AK_Success;
	if (const auto* StreamableFileStateInfo = State->GetStreamableFileStateInfo())
	{
		StreamableFileStateInfo->GetFileDescCopy(OutFileDesc);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: Could not get AkFileDesc."), InShortId);
		Result = AK_UnknownFileError;
	}

	return Result;
}

void FWwiseFileHandlerBase::CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState)
{
	return DecrementFileStateUseAsync(InShortId, &InFileState, EWwiseFileStateOperationOrigin::Streaming, []{});
}

void FWwiseFileHandlerBase::IncrementFileStateUseAsync(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin,
	FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback)
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
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		if (const auto* StatePtr = FileStatesById.Find(InShortId))
		{
			State = *StatePtr;
		}
	}

	if (!State.IsValid())
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_Write);
		if (LIKELY(!FileStatesById.Contains(InShortId)))
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Creating new State for %s %" PRIu32), GetManagingTypeName(), InShortId);
			State = InCreate();
			if (LIKELY(State.IsValid()))
			{
				FileStatesById.Add(InShortId, State);
			}
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
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		const auto* StatePtr = FileStatesById.Find(InShortId);
		if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("Could not find state for for %s %" PRIu32), GetManagingTypeName(), InShortId);
			InCallback();
			return;
		}
		InFileState = StatePtr->Get();
	}

	InFileState->DecrementCountAsync(InOperationOrigin, [this, InShortId, InFileState, InOperationOrigin](FDecrementStateCallback&& InCallback) mutable
	{
		// File state deletion request
		FileHandlerExecutionQueue.Async([this, InShortId, InFileState, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
		{
			OnDeleteState(InShortId, *InFileState, InOperationOrigin, MoveTemp(InCallback));
		});
	}, MoveTemp(InCallback));
}

void FWwiseFileHandlerBase::OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_Write);
		if (!InFileState.CanDelete())
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OnDeleteState %s %" PRIu32 ": Cannot delete State. Probably re-loaded between deletion request and now."),
				GetManagingTypeName(), InShortId);
			InCallback();
			return;
		}

		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OnDeleteState %s %" PRIu32 ": Deleting."), GetManagingTypeName(), InShortId);
		const auto RemovalCount = FileStatesById.Remove(InShortId);		// WARNING: This will very probably delete InFileState reference. Do not use the File State from that point!

		UE_CLOG(RemovalCount != 1, LogWwiseFileHandler, Error, TEXT("Removing a state for %s %" PRIu32 ", ended up deleting %" PRIi32 " states."),
			GetManagingTypeName(), InShortId, RemovalCount);
	}

	InCallback();
}
