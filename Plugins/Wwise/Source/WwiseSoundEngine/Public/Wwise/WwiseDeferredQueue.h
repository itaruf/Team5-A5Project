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

#pragma once

#include "Wwise/WwiseExecutionQueue.h"

enum class WWISESOUNDENGINE_API EWwiseDeferredAsyncResult
{
	Done,
	Retry
};

struct WWISESOUNDENGINE_API FWwiseDeferredQueue
{
	using FAsyncFunction = TUniqueFunction<EWwiseDeferredAsyncResult()>;
	using FSyncFunction = FWwiseExecutionQueue::FBasicFunction;

	FWwiseDeferredQueue();
	~FWwiseDeferredQueue();

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where a task is started, and the function is run asynchronously.
	 * @param InFunction The function to execute
	 *
	 * For most uses, there is no time sensitivity to a deferred operation. Every single deferred Async operation are run sequentially, however,
	 * they might be executed at the same time than the Sync and Game operations. Also, these will probably be called in a non-game thread, so you are
	 * responsible to properly bound your code if this causes issues.
	 *
	 * The return value of the passed function should be "true" if it was properly executed, whether there was an error or not. The return value
	 * should be "false" if it should be deferred again.
	 */
	void AsyncDefer(FAsyncFunction&& InFunction);

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where the function is immediately called in the Run() or Wait() thread.
	 * @param InFunction The function to execute, synchronous to the Run() thread.
	 *
	 * @warning Since this function will be called synchronously to the Run() call, if the running thread is time-sensitive, it can cause glitches
	 * or hitches. In most cases, you should use DeferAsync or DeferGame.
	 * 
	 * This is required for time-sensitive issues, such as waiting for an operation that absolutely needs to be done before the Run() thread gains
	 * control back.
	 */
	void SyncDefer(FSyncFunction&& InFunction);

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where the function is subsequently run on the Game Thread.
	 * @param InFunction The function to execute
	 *
	 * For most game uses, there is no time sensitivity to a deferred operation, but you might want to notify game objects when an event occurs.
	 *
	 * Every single deferred Game operation are run sequentially, however, they might be executed at the same time than the other Async and Sync
	 * operations.
	 */
	void GameDefer(FSyncFunction&& InFunction);


	void Run();
	void Wait();
	bool IsEmpty() const { return AsyncOpQueue.IsEmpty() && SyncOpQueue.IsEmpty() && GameOpQueue.IsEmpty(); }

protected:
	FWwiseExecutionQueue AsyncExecutionQueue;
	FWwiseExecutionQueue GameExecutionQueue;

	using FAsyncOps = TQueue<FAsyncFunction, EQueueMode::Mpsc>;
	using FSyncOps = TQueue<FSyncFunction, EQueueMode::Mpsc>;
	FAsyncOps AsyncOpQueue;
	FSyncOps SyncOpQueue;
	FSyncOps GameOpQueue;

private:
	void AsyncExec();
	void SyncExec();
	void GameExec();
};
