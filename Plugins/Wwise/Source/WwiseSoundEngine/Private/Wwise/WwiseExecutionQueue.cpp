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

#include "Wwise/WwiseExecutionQueue.h"
#include "Async/Async.h"
#include "HAL/Event.h"

FWwiseExecutionQueue::FWwiseExecutionQueue(ENamedThreads::Type InThread) :
	Thread(InThread),
	WorkerState(EWorkerState::Stopped),
	OpQueue()
{}

FWwiseExecutionQueue::~FWwiseExecutionQueue()
{
	Close();
}

bool FWwiseExecutionQueue::Async(FBasicFunction&& InFunction)
{
	if (UNLIKELY(!OpQueue.Enqueue(MoveTemp(InFunction))))
	{
		return false;
	}
	return StartWorkerIfNeeded();
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

void FWwiseExecutionQueue::Close()
{
	if (!TrySetStoppedWorkerToClosed())
	{
		for (auto State = WorkerState.Load(EMemoryOrder::Relaxed);
			State != EWorkerState::Closed;
			State = WorkerState.Load(EMemoryOrder::Relaxed))
		{
			if (State != EWorkerState::Closing)
			{
				AsyncWait([this]
				{
					if (OpQueue.IsEmpty())
					{
						TrySetRunningWorkerToClosing();
					}
				});
			}
		}
	}
}

bool FWwiseExecutionQueue::IsBeingClosed() const
{
	const auto State = WorkerState.Load(EMemoryOrder::Relaxed);
	return UNLIKELY(State == EWorkerState::Closed || State == EWorkerState::Closing);
}

bool FWwiseExecutionQueue::IsClosed() const
{
	const auto State = WorkerState.Load(EMemoryOrder::Relaxed);
	return State == EWorkerState::Closed;
}

bool FWwiseExecutionQueue::StartWorkerIfNeeded()
{
	if (TrySetStoppedWorkerToRunning())
	{
		AsyncTask(Thread, [this]
		{
			Work();
		});
		return true;
	}
	else if (UNLIKELY(IsBeingClosed()))
	{
		return false;
	}
	else
	{
		return true;
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
		if (LIKELY(TrySetRunningWorkerToStopped()))
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
		else if (LIKELY(TrySetClosingWorkerToClosed()))
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

bool FWwiseExecutionQueue::TrySetStoppedWorkerToClosed()
{
	EWorkerState ThreadStopped = EWorkerState::Stopped;
	return WorkerState.CompareExchange(ThreadStopped, EWorkerState::Closed);
}

bool FWwiseExecutionQueue::TrySetRunningWorkerToStopped()
{
	EWorkerState ThreadRunning = EWorkerState::Running;
	return WorkerState.CompareExchange(ThreadRunning, EWorkerState::Stopped);
}

bool FWwiseExecutionQueue::TrySetRunningWorkerToClosing()
{
	EWorkerState ThreadRunning = EWorkerState::Running;
	return WorkerState.CompareExchange(ThreadRunning, EWorkerState::Closing);
	// Warning: Try not to do operations past this method returning "true". There's a slight chance "this" might be deleted!
}

bool FWwiseExecutionQueue::TrySetClosingWorkerToClosed()
{
	EWorkerState ThreadClosing = EWorkerState::Closing;
	return WorkerState.CompareExchange(ThreadClosing, EWorkerState::Closed);
	// Warning: NEVER do operations past this method returning "true". "this" is probably deleted!
}
