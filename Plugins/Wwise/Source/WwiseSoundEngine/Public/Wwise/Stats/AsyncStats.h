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

#include "Async/Async.h"
#include "Stats/Stats.h"

// Unreal Stat system assumes it runs on an Unreal thread. A lot of low-level I/O doesn't follow that assumption.

#if STATS
#define ASYNC_LAMBDA_STAT(...) Async(EAsyncExecution::TaskGraph, __VA_ARGS__)
#define ASYNC_OP_STAT(...) ASYNC_LAMBDA_STAT([]__VA_ARGS__)

class FWwiseAsyncCycleCounter
{
public:
	FName StatName;
	FStatMessage StartStatMessage;

	FORCEINLINE_STATS static FName GetName(TStatId InStatId)
	{
		FMinimalName StatMinimalName = InStatId.GetMinimalName(EMemoryOrder::Relaxed);
		if (UNLIKELY(StatMinimalName.IsNone()))
		{
			return {};
		}
		return MinimalNameToName(StatMinimalName);
	}

	FORCEINLINE_STATS static FStatMessage GetStartMessage(FName InStatName)
	{
		if (UNLIKELY(InStatName.IsNone()))
		{
			return {};
		}
		return FStatMessage(InStatName, EStatOperation::CycleScopeStart);
	}

	void Stop()
	{
		if (StatName.IsNone())
		{
			return;
		}

		FName Name;
		Swap(Name, StatName);

		FStatMessage EndStatMessage(Name, EStatOperation::CycleScopeEnd);
		ASYNC_LAMBDA_STAT([
			Name = MoveTemp(Name),
			StartStatMessage = MoveTemp(StartStatMessage),
			EndStatMessage = MoveTemp(EndStatMessage)]
		{
			FThreadStats* Stats = FThreadStats::GetThreadStats();
			Stats->AddStatMessage(StartStatMessage);
			Stats->AddStatMessage(EndStatMessage);
			Stats->Flush();
		});
	}

	FWwiseAsyncCycleCounter(TStatId InStatId) :
		StatName(GetName(InStatId)),
		StartStatMessage(GetStartMessage(StatName))
	{
	}

	~FWwiseAsyncCycleCounter()
	{
		Stop();
	}
};
#else
#define ASYNC_LAMBDA_STAT(...) (void)0
#define ASYNC_OP_STAT(...) (void)0


class FWwiseAsyncCycleCounter
{
public:
	void Stop()
	{
	}

	FWwiseAsyncCycleCounter(TStatId InStatId)
	{
	}

	~FWwiseAsyncCycleCounter()
	{
	}
};
#endif

#define ASYNC_INC_DWORD_STAT(x) ASYNC_OP_STAT({ INC_DWORD_STAT(x); })
#define ASYNC_DEC_DWORD_STAT(x) ASYNC_OP_STAT({ DEC_DWORD_STAT(x); })
#define ASYNC_INC_FLOAT_STAT_BY(x, y) ASYNC_LAMBDA_STAT([Value = y]{ INC_FLOAT_STAT_BY(x, Value); })