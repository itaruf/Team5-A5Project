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

#pragma once

#include "CoreMinimal.h"
#include "Async/Async.h"
#include "Containers/Queue.h"

struct WWISESOUNDENGINE_API FWwiseExecutionQueue
{
	using FBasicFunction = TUniqueFunction<void()>;

	enum ETimedResult
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

	void Stop();

private:
	enum class WWISESOUNDENGINE_API EWorkerState
	{
		Stopped,
		Running,
		Exiting
	};
	TAtomic<EWorkerState> WorkerState;

	using FOpQueue = TQueue<FBasicFunction, EQueueMode::Mpsc>;
	FOpQueue OpQueue;

	const ENamedThreads::Type Thread;

	void StartWorkerIfNeeded();
	void Work();
	bool StopWorkerIfDone();
	void ProcessWork();
	bool TrySetStoppedWorkerToRunning();
	bool TrySetRunningWorkerToStopped();
	bool TrySetExitingWorkerToStopped();
};
