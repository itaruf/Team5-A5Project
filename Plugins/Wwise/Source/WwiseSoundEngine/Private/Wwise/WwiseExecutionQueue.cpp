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

#include "Wwise/WwiseExecutionQueue.h"
#include "Async/Async.h"
#include "HAL/Event.h"

FWwiseExecutionQueue::FWwiseExecutionQueue(ENamedThreads::Type InThread) :
	WorkerState(EWorkerState::Stopped),
	OpQueue(),
	Thread(InThread)
{}

FWwiseExecutionQueue::~FWwiseExecutionQueue()
{
	Stop();
}

bool FWwiseExecutionQueue::Async(FBasicFunction&& InFunction)
{
	if (UNLIKELY(!OpQueue.Enqueue(MoveTemp(InFunction))))
	{
		return false;
	}
	StartWorkerIfNeeded();
	return true;
}

bool FWwiseExecutionQueue::AsyncWait(FBasicFunction&& InFunction)
{
	FEventRef Event(EEventMode::ManualReset);
	if (UNLIKELY(!Async([&Event, &InFunction] {
		InFunction();
		Event->Trigger();
	})))
	{
		return false;
	}
	Event->Wait();
	return true;
}

void FWwiseExecutionQueue::Async(FTimedCallback&& InCallback, FTimedFunction&& InFunction, FTimespan InDeadline)
{
	if (InDeadline == NoTimeLimit())
	{
		if (UNLIKELY(!Async([InCallback = MoveTemp(InCallback), InFunction = MoveTemp(InFunction)]{
			InCallback(InFunction(NoTimeLimit()));
		})))
		{
			InCallback(ETimedResult::Failure);
		}
	}
	else
	{
		if (UNLIKELY(!Async([InCallback = MoveTemp(InCallback), InFunction = MoveTemp(InFunction), InDeadline]{
			InCallback(InFunction(InDeadline));
		})))
		{
			InCallback(ETimedResult::Failure);
		}
	}
}

bool FWwiseExecutionQueue::AsyncWithCallback(FResultCallback&& InCallback, FResultFunction&& InFunction)
{
	return Async([InCallback = MoveTemp(InCallback), InFunction = MoveTemp(InFunction)]{
		InCallback(InFunction());
	});
}

void FWwiseExecutionQueue::Stop()
{
	if (WorkerState.Load() == EWorkerState::Running)
	{
		AsyncWait([this]
		{
			WorkerState.Store(EWorkerState::Exiting);
		});
		while (WorkerState.Load(EMemoryOrder::Relaxed) != EWorkerState::Stopped) {}
	}
}

void FWwiseExecutionQueue::StartWorkerIfNeeded()
{
	if (TrySetStoppedWorkerToRunning())
	{
		AsyncTask(Thread, [this]
		{
			Work();
		});
	}
}

void FWwiseExecutionQueue::Work()
{
	do
	{
		ProcessWork();
		
	}
	while (!StopWorkerIfDone());
}

bool FWwiseExecutionQueue::StopWorkerIfDone()
{
	if (OpQueue.IsEmpty())
	{
		if (TrySetRunningWorkerToStopped())
		{
			// If we have a new operation in the queue, we might have to start again
			if (!OpQueue.IsEmpty()
				&& TrySetStoppedWorkerToRunning())
			{
				// A new operation got added while we weren't looking. Check if it's still true.
				// Another worker might have started and stopped between those calls - but we still managed to request our thread to be the running one.
				return false;
			}
			else
			{
				// We don't have operations queued or we weren't the worker that was able to start. Done.
				return true;
			}
		}
		else if (TrySetExitingWorkerToStopped())
		{
			// We were exiting and we don't have operations anymore. Immediately return, as our worker is not valid at this point.
			// Don't do any operations here!
			return true;
		}
		else
		{
			checkf(false, TEXT("Worker is stopped, but we haven't stopped it ourselves."));
			return true;
		}
	}
	return false;
}

void FWwiseExecutionQueue::ProcessWork()
{
	FBasicFunction Op;
	if (OpQueue.Dequeue(Op))
	{
		Op();
	}
}

bool FWwiseExecutionQueue::TrySetStoppedWorkerToRunning()
{
	EWorkerState ThreadStopped = EWorkerState::Stopped;
	return WorkerState.CompareExchange(ThreadStopped, EWorkerState::Running);

}

bool FWwiseExecutionQueue::TrySetRunningWorkerToStopped()
{
	EWorkerState ThreadRunning = EWorkerState::Running;
	return WorkerState.CompareExchange(ThreadRunning, EWorkerState::Stopped);
	// Warning: Try not to do operations past this method returning "true". There's a slight chance "this" might be deleted!
}

bool FWwiseExecutionQueue::TrySetExitingWorkerToStopped()
{
	EWorkerState ThreadExiting = EWorkerState::Exiting;
	return WorkerState.CompareExchange(ThreadExiting, EWorkerState::Stopped);
	// Warning: NEVER do operations past this method returning "true". "this" is probably deleted!
}
