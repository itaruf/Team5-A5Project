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

#include "Wwise/WwiseDeferredQueue.h"

#include "Wwise/Stats/SoundEngine.h"

FWwiseDeferredQueue::FWwiseDeferredQueue() :
	GameExecutionQueue(ENamedThreads::GameThread)
{
}

FWwiseDeferredQueue::~FWwiseDeferredQueue()
{
	if (!IsEmpty())
	{
		Wait();
		UE_CLOG(UNLIKELY(!IsEmpty()), LogWwiseSoundEngine, Error, TEXT("Still operations in queue while deleting Deferred Queue"));
	}
}

void FWwiseDeferredQueue::AsyncDefer(FAsyncFunction&& InFunction)
{
	AsyncOpQueue.Enqueue(MoveTemp(InFunction));
}

void FWwiseDeferredQueue::SyncDefer(FSyncFunction&& InFunction)
{
	SyncOpQueue.Enqueue(MoveTemp(InFunction));
}

void FWwiseDeferredQueue::GameDefer(FSyncFunction&& InFunction)
{
	GameOpQueue.Enqueue(MoveTemp(InFunction));
}

void FWwiseDeferredQueue::Run()
{
	if (!AsyncOpQueue.IsEmpty())
	{
		AsyncExecutionQueue.Async([this]() mutable
		{
			AsyncExec();
		});
	}
	if (!GameOpQueue.IsEmpty())
	{
		GameExecutionQueue.Async([this]() mutable
		{
			GameExec();
		});
	}
	if (!SyncOpQueue.IsEmpty())
	{
		SyncExec();
	}
}

void FWwiseDeferredQueue::Wait()
{
	if (!AsyncOpQueue.IsEmpty())
	{
		AsyncExecutionQueue.AsyncWait([this]() mutable
		{
			AsyncExec();
		});
	}
	if (!GameOpQueue.IsEmpty())
	{
		GameExecutionQueue.AsyncWait([this]() mutable
		{
			GameExec();
		});
	}
	if (!SyncOpQueue.IsEmpty())
	{
		SyncExec();
	}
}

void FWwiseDeferredQueue::AsyncExec()
{
	bool bDone = false;
	AsyncDefer([&bDone]() mutable
	{
		bDone = true;
		return EWwiseDeferredAsyncResult::Done;
	});

	while (!bDone)
	{
		FAsyncFunction Func;
		const bool bResult = AsyncOpQueue.Dequeue(Func);
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("No Result dequeuing Deferred Queue"));
			break;
		}
		if (Func() == EWwiseDeferredAsyncResult::Retry)
		{
			AsyncDefer(MoveTemp(Func));
		}
	}
}

void FWwiseDeferredQueue::SyncExec()
{
	FSyncFunction Func;
	while (SyncOpQueue.Dequeue(Func))
	{
		Func();
	}
}

void FWwiseDeferredQueue::GameExec()
{
	bool bDone = false;
	GameDefer([&bDone]() mutable
	{
		bDone = true;
		return true;
	});

	while (!bDone)
	{
		FSyncFunction Func;
		const bool bResult = GameOpQueue.Dequeue(Func);
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("No Result dequeuing Deferred Queue"));
			break;
		}
		Func();
	}
}
