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

#include "CoreMinimal.h"
#include "Async/Async.h"
#include "Containers/Queue.h"

struct WWISESOUNDENGINE_API FWwiseExecutionQueue
{
	ENamedThreads::Type Thread;

	using FBasicFunction = TUniqueFunction<void()>;

	enum class WWISESOUNDENGINE_API ETimedResult
	{
		Success,
		Failure,
		Timeout
	};
	using FTimedFunction = TUniqueFunction<ETimedResult(FTimespan InDeadline)>;
	using FTimedCallback = TUniqueFunction<void(ETimedResult bInResult)>;

	using FResultFunction = TUniqueFunction<bool()>;
	using FResultCallback = TUniqueFunction<void(bool bInResult)>;

	static FTimespan Now() { return FDateTime::UtcNow().GetTicks(); }
	static FTimespan NoTimeLimit() { return FTimespan::MaxValue(); }
	
	FWwiseExecutionQueue(ENamedThreads::Type InThread = ENamedThreads::AnyThread);
	~FWwiseExecutionQueue();

	bool Async(FBasicFunction&& InFunction);
	bool AsyncWait(FBasicFunction&& InFunction);
	void Async(FTimedCallback&& InCallback, FTimedFunction&& InFunction, FTimespan InDeadline = NoTimeLimit());
	bool AsyncWithCallback(FResultCallback&& InCallback, FResultFunction&& InFunction);

	void Close();
	bool IsBeingClosed() const;
	bool IsClosed() const;

private:
	enum class WWISESOUNDENGINE_API EWorkerState
	{
		Stopped,
		Running,
		Closing,
		Closed
	};
	TAtomic<EWorkerState> WorkerState;

	using FOpQueue = TQueue<FBasicFunction, EQueueMode::Mpsc>;
	FOpQueue OpQueue;

	bool StartWorkerIfNeeded();
	void Work();
	bool StopWorkerIfDone();
	void ProcessWork();
	bool TrySetStoppedWorkerToRunning();
	bool TrySetStoppedWorkerToClosed();
	bool TrySetRunningWorkerToStopped();
	bool TrySetRunningWorkerToClosing();
	bool TrySetClosingWorkerToClosed();
};
