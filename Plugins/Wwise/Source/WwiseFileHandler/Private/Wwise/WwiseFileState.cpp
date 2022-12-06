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

#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/Stats/AsyncStats.h"

#include <inttypes.h>

#include "Wwise/WwiseStreamableFileStateInfo.h"

FWwiseFileState::~FWwiseFileState()
{
	UE_CLOG(!FileStateExecutionQueue.IsClosed(), LogWwiseFileHandler, Error, TEXT("Closing the File State without closing the Execution Queue."));
}

void FWwiseFileState::IncrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin,
	FIncrementCountCallback&& InCallback)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerStateOperationLatency));
	FileStateExecutionQueue.Async([this, InOperationOrigin, OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
		IncrementCount(InOperationOrigin, [OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback)](bool bInResult) mutable
		{
			OpCycleCounter.Stop();
			DEC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
			InCallback(bInResult);
		});
	});
}

void FWwiseFileState::DecrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerStateOperationLatency));

	FileStateExecutionQueue.Async([this, InOperationOrigin, OpCycleCounter = MoveTemp(OpCycleCounter), InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
		DecrementCount(InOperationOrigin, MoveTemp(InDeleteState), [OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback)]() mutable
		{
			OpCycleCounter.Stop();
			DEC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
			InCallback();
		});
	});
}

bool FWwiseFileState::CanDelete() const
{
	return State == EState::Closed && LoadCount == 0;
}

FWwiseFileState::FWwiseFileState():
	LoadCount(0),
	StreamingCount(0),
	State(EState::Closed)
{
}

void FWwiseFileState::Term()
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Term %s file state %" PRIu32"."), GetManagingTypeName(), GetShortId());
	UE_CLOG(!IsEngineExitRequested() && UNLIKELY(State != EState::Closed), LogWwiseFileHandler, Warning, TEXT("%s State: Term unclosed file state %" PRIu32 ". Leaking."), GetManagingTypeName(), GetShortId());
	UE_CLOG(IsEngineExitRequested() && State != EState::Closed, LogWwiseFileHandler, VeryVerbose, TEXT("%s State: Term unclosed file state %" PRIu32 " at exit. Leaking."), GetManagingTypeName(), GetShortId());
	UE_CLOG(LoadCount != 0, LogWwiseFileHandler, Error, TEXT("Term %s file state %" PRIu32 " when there are still %d load count"), GetManagingTypeName(), GetShortId(), LoadCount);

	FileStateExecutionQueue.Close();
}

void FWwiseFileState::IncrementCount(EWwiseFileStateOperationOrigin InOperationOrigin,
                                     FIncrementCountCallback&& InCallback)
{
	IncrementLoadCount(InOperationOrigin);

	IncrementCountOpen(InOperationOrigin, MoveTemp(InCallback));
}

void FWwiseFileState::IncrementCountOpen(EWwiseFileStateOperationOrigin InOperationOrigin,
	FIncrementCountCallback&& InCallback)
{
	if (State == EState::Closing)
	{
		// We are currently closing asynchronously. Meaning this is a lengthy operation. Wait until the next End global callback.
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("IncrementCountOpen %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			IncrementCountDone(InOperationOrigin, MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": Closing -> WillReopen"),
						GetManagingTypeName(), GetShortId());
		State = EState::WillReopen;

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": Waiting for deferred Closing file. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": Retrying open"),
								GetManagingTypeName(), GetShortId());
				IncrementCountOpen(InOperationOrigin, MoveTemp(InCallback));			// Call ourselves back
			});
			return EWwiseDeferredAsyncResult::Done;
		});
		return;
	}

	if (State == EState::CanReopen)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": CanReopen -> Closed (post-close)"),
						GetManagingTypeName(), GetShortId());
		State = EState::Closed;
	}

	if (!CanOpenFile())
	{
		IncrementCountLoad(InOperationOrigin, MoveTemp(InCallback));					// Continue
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": Closed -> Opening"),
					GetManagingTypeName(), GetShortId());
	State = EState::Opening;

	OpenFile([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
	{
		IncrementCountLoad(InOperationOrigin, MoveTemp(InCallback));					// Continue
	});
}

void FWwiseFileState::IncrementCountLoad(EWwiseFileStateOperationOrigin InOperationOrigin,
	FIncrementCountCallback&& InCallback)
{
	if (State == EState::Unloading)
	{
		// We are currently unloading asynchronously. Meaning this is a lengthy operation. Wait until the next End global callback.
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("IncrementCountLoad %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			IncrementCountDone(InOperationOrigin, MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountLoad %s %" PRIu32 ": Unloading -> WillReload"),
						GetManagingTypeName(), GetShortId());
		State = EState::WillReload;

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountLoad %s %" PRIu32 ": Waiting for deferred Unloading. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountLoad %s %" PRIu32 ": Retrying open"),
								GetManagingTypeName(), GetShortId());
				IncrementCountOpen(InOperationOrigin, MoveTemp(InCallback));			// Restart the op from start
			});
			return EWwiseDeferredAsyncResult::Done;
		});
		return;
	}

	if (State == EState::CanReload)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountLoad %s %" PRIu32 ": CanReload -> Opened (post-unload)"),
						GetManagingTypeName(), GetShortId());
		State = EState::Opened;
	}

	if (!CanLoadInSoundEngine())
	{
		IncrementCountDone(InOperationOrigin, MoveTemp(InCallback));					// Continue
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountOpen %s %" PRIu32 ": Opened -> Loading"),
					GetManagingTypeName(), GetShortId());
	State = EState::Loading;

	LoadInSoundEngine([this, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
	{
		IncrementCountDone(InOperationOrigin, MoveTemp(InCallback));					// Continue
	});
}

void FWwiseFileState::IncrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin,
	FIncrementCountCallback&& InCallback)
{
	bool bResult;
	if (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming)
	{
		bResult = (State == EState::Loaded);
		UE_CLOG(UNLIKELY(!bResult), LogWwiseFileHandler, Warning, TEXT("IncrementCountDone %s %" PRIu32 ": Could not load file for IO Hook streaming."),
			GetManagingTypeName(), GetShortId());
	}
	else
	{
		bResult = (State == EState::Loaded)
			|| (State == EState::Opened && !CanLoadInSoundEngine())
			|| (State == EState::Closed && !CanOpenFile());
		UE_CLOG(UNLIKELY(!bResult), LogWwiseFileHandler, Warning, TEXT("IncrementCountDone %s %" PRIu32 ": Could not open file for asset loading."),
			GetManagingTypeName(), GetShortId());
	}

	UE_CLOG(LIKELY(bResult), LogWwiseFileHandler, VeryVerbose, TEXT("IncrementCountDone %s %" PRIu32 ": Done incrementing."),
		GetManagingTypeName(), GetShortId());
	InCallback(bResult);
}

void FWwiseFileState::DecrementCount(EWwiseFileStateOperationOrigin InOperationOrigin,
                                  FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	if (UNLIKELY(LoadCount == 0))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("DecrementCount %s %" PRIu32 ": File State is already closed."), GetManagingTypeName(), GetShortId());
		InCallback();
		return;
	}

	DecrementLoadCount(InOperationOrigin);

	DecrementCountUnload(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));
}

void FWwiseFileState::DecrementCountUnload(EWwiseFileStateOperationOrigin InOperationOrigin,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	if (State == EState::Unloading || State == EState::WillReload || State == EState::CanReload)
	{
		// We are currently unloading asynchronously. Meaning this is a lengthy operation. Wait until the next End global callback.
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("DecrementCountUnload %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": UnloadFromSoundEngine deferred by another user. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": Retrying unload"),
								GetManagingTypeName(), GetShortId());
				DecrementCountUnload(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));			// Call ourselves back
			});
			return EWwiseDeferredAsyncResult::Done;
		});
		return;
	}

	if (!CanUnloadFromSoundEngine())
	{
		DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));					// Continue
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": -> Unloading"),
					GetManagingTypeName(), GetShortId());
	State = EState::Unloading;

	UnloadFromSoundEngine([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)](EWwiseDeferredAsyncResult InDefer) mutable
	{
		if (LIKELY(InDefer == EWwiseDeferredAsyncResult::Done))
		{
			DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Continue
			return;
		}

		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("DecrementCountUnload %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("DecrementCountUnload %s %" PRIu32 ": UnloadFromSoundEngine deferred. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("DecrementCountUnload %s %" PRIu32 ": Processing deferred Unload."),
					GetManagingTypeName(), GetShortId());
				
				if (UNLIKELY(State == EState::WillReload))
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": Another user needs this to be kept loaded."),
							GetManagingTypeName(), GetShortId());
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": WillReload -> Loaded"),
									GetManagingTypeName(), GetShortId());
					State = EState::Loaded;
					DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Skip all
				}
				else if (UNLIKELY(State != EState::Unloading))
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": State got changed. Not unloading anymore."),
							GetManagingTypeName(), GetShortId());
					DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Continue
				}
				else
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountUnload %s %" PRIu32 ": Unloading -> Loaded (retry)"),
									GetManagingTypeName(), GetShortId());
					State = EState::Loaded;
					DecrementCountUnload(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Call ourselves back
				}
			});
			return EWwiseDeferredAsyncResult::Done;
		});
	});
}

void FWwiseFileState::DecrementCountClose(EWwiseFileStateOperationOrigin InOperationOrigin,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	if (State == EState::Closing || State == EState::WillReopen || State == EState::CanReopen)
	{
		// We are currently closing asynchronously. Meaning this is a lengthy operation. Wait until the next End global callback.
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("DecrementCountClose %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": CloseFile deferred by another user. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": Retrying close"),
								GetManagingTypeName(), GetShortId());
				DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));			// Call ourselves back
			});
			return EWwiseDeferredAsyncResult::Done;
		});
		return;
	}

	if (!CanCloseFile())
	{
		DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));					// Continue
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": -> Closing"),
					GetManagingTypeName(), GetShortId());
	State = EState::Closing;

	CloseFile([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)](EWwiseDeferredAsyncResult InDefer) mutable
	{
		if (LIKELY(InDefer == EWwiseDeferredAsyncResult::Done))
		{
			DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Continue
			return;
		}

		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("DecrementCountClose %s %" PRIu32 ": GlobalCallbacks unavailable. Giving up."),
				GetManagingTypeName(), GetShortId());
			DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Skip all
			return;
		}

		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("DecrementCountClose %s %" PRIu32 ": CloseFile deferred. Wait until End callback."),
			GetManagingTypeName(), GetShortId());
		GlobalCallbacks->EndAsync([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			FileStateExecutionQueue.Async([this, InOperationOrigin, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("DecrementCountClose %s %" PRIu32 ": Processing deferred Close."),
					GetManagingTypeName(), GetShortId());
				
				if (UNLIKELY(State == EState::WillReopen))
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": Another user needs this to be kept open."),
							GetManagingTypeName(), GetShortId());
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": WillReopen -> Opened"),
									GetManagingTypeName(), GetShortId());
					State = EState::Opened;
					DecrementCountDone(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Skip all
				}
				else if (UNLIKELY(State != EState::Closing))
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": State got changed. Not closing anymore."),
							GetManagingTypeName(), GetShortId());
					DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Continue
				}
				else
				{
					UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountClose %s %" PRIu32 ": Closing -> Opened (retry)"),
									GetManagingTypeName(), GetShortId());
					State = EState::Opened;
					DecrementCountClose(InOperationOrigin, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Call ourselves back
				}
			});
			return EWwiseDeferredAsyncResult::Done;
		});
	});
}

void FWwiseFileState::DecrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	if (CanDelete())
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountDone %s %" PRIu32 ": Done decrementing. Deleting state."),
				GetManagingTypeName(), GetShortId());
		InDeleteState(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("DecrementCountDone %s %" PRIu32 ": Done decrementing."),
				GetManagingTypeName(), GetShortId());
		InCallback();
	}
}

void FWwiseFileState::IncrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin)
{
	const bool bIncrementStreamingCount = (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming);

	if (bIncrementStreamingCount) ++StreamingCount;
	++LoadCount;

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " LoadCount %d %sStreamingCount %d"),
	       GetManagingTypeName(), GetShortId(), LoadCount, bIncrementStreamingCount ? TEXT("++") : TEXT(""), StreamingCount);
}

bool FWwiseFileState::CanOpenFile() const
{
	return State == EState::Closed && LoadCount > 0;
}

void FWwiseFileState::OpenFileSucceeded(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Opening))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Done opening %s %" PRIu32 " while not in Opening state"), GetManagingTypeName(), GetShortId());
		InCallback();
		return;
	}
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Opening -> Opened"), GetManagingTypeName(), GetShortId());
	State = EState::Opened;
	InCallback();
}

void FWwiseFileState::OpenFileFailed(FOpenFileCallback&& InCallback)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
	if (UNLIKELY(State != EState::Opening))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Failed opening %s %" PRIu32 " while not in Opening state"), GetManagingTypeName(), GetShortId());
		InCallback();
		return;
	}
	UE_LOG(LogWwiseFileHandler, Warning, TEXT("State %s %" PRIu32 " Opening Failed -> Closed"), GetManagingTypeName(), GetShortId());
	State = EState::Closed;
	InCallback();
}

bool FWwiseFileState::CanLoadInSoundEngine() const
{
	return State == EState::Opened && (!IsStreamedState() || StreamingCount > 0);
}

void FWwiseFileState::LoadInSoundEngineSucceeded(FLoadInSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Loading))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Done loading %s %" PRIu32 " while not in Loading state"), GetManagingTypeName(), GetShortId());
		InCallback();
		return;
	}
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Loading -> Loaded"), GetManagingTypeName(), GetShortId());
	State = EState::Loaded;
	InCallback();
}

void FWwiseFileState::LoadInSoundEngineFailed(FLoadInSoundEngineCallback&& InCallback)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
	if (UNLIKELY(State != EState::Loading))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Failed loading %s %" PRIu32 " while not in Loading state"), GetManagingTypeName(), GetShortId());
		InCallback();
		return;
	}
	UE_LOG(LogWwiseFileHandler, Warning, TEXT("State %s %" PRIu32 " Loading Failed -> Opened"), GetManagingTypeName(), GetShortId());
	State = EState::Opened;
	InCallback();
}

void FWwiseFileState::DecrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin)
{
	const bool bDecrementStreamingCount = (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming);

	if (bDecrementStreamingCount) --StreamingCount;
	--LoadCount;

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " --LoadCount %d %sStreamingCount %d"),
	       GetManagingTypeName(), GetShortId(), LoadCount, bDecrementStreamingCount ? TEXT("--") : TEXT(""), StreamingCount);
}

bool FWwiseFileState::CanUnloadFromSoundEngine() const
{
	return State == EState::Loaded && ((IsStreamedState() && StreamingCount == 0) || (!IsStreamedState() && LoadCount == 0));
}

void FWwiseFileState::UnloadFromSoundEngineDone(FUnloadFromSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Done unloading %s %" PRIu32 " while not in Unloading state"), GetManagingTypeName(), GetShortId());
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}
	if (LIKELY(State == EState::Unloading))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Unloading -> Opened"), GetManagingTypeName(), GetShortId());
		State = EState::Opened;
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " WillReload -> CanReload"), GetManagingTypeName(), GetShortId());
		State = EState::CanReload;
	}
	InCallback(EWwiseDeferredAsyncResult::Done);
}

void FWwiseFileState::UnloadFromSoundEngineToClosedFile(FUnloadFromSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Done unloading %s %" PRIu32 " while not in Unloading state"), GetManagingTypeName(), GetShortId());
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}
	if (LIKELY(State == EState::Unloading))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Unloading -...-> Closed"), GetManagingTypeName(), GetShortId());
		State = EState::Closed;
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " WillReload -> CanReload -> CanReopen"), GetManagingTypeName(), GetShortId());
		State = EState::CanReopen;
	}
	InCallback(EWwiseDeferredAsyncResult::Done);
}


void FWwiseFileState::UnloadFromSoundEngineDefer(FUnloadFromSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Deferring unloading %s %" PRIu32 " while not in Unloading state"), GetManagingTypeName(), GetShortId());
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}
	if (UNLIKELY(State == EState::WillReload))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " WillReload -> Loaded"), GetManagingTypeName(), GetShortId());
		State = EState::Loaded;
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Deferring Unload"), GetManagingTypeName(), GetShortId());
	InCallback(EWwiseDeferredAsyncResult::Retry);
}

bool FWwiseFileState::CanCloseFile() const
{
	return State == EState::Opened && LoadCount == 0;
}

void FWwiseFileState::CloseFileDone(FCloseFileCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Closing && State != EState::WillReopen))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Done closing %s %" PRIu32 " while not in Closing state"), GetManagingTypeName(), GetShortId());
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}
	if (LIKELY(State == EState::Closing))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Closing -> Closed"), GetManagingTypeName(), GetShortId());
		State = EState::Closed;
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " WillReopen -> CanReopen"), GetManagingTypeName(), GetShortId());
		State = EState::CanReopen;
	}
	InCallback(EWwiseDeferredAsyncResult::Done);
}

void FWwiseFileState::CloseFileDefer(FCloseFileCallback&& InCallback)
{
	if (UNLIKELY(State != EState::Closing && State != EState::WillReopen))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Deferring closing %s %" PRIu32 " while not in Closing state"), GetManagingTypeName(), GetShortId());
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}
	if (UNLIKELY(State == EState::WillReopen))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " WillReopen -> Opened"), GetManagingTypeName(), GetShortId());
		State = EState::Opened;
		InCallback(EWwiseDeferredAsyncResult::Done);
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("State %s %" PRIu32 " Deferring Close"), GetManagingTypeName(), GetShortId());
	InCallback(EWwiseDeferredAsyncResult::Retry);
}
