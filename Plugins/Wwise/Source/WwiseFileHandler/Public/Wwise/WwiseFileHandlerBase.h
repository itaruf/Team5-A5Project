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

#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/WwiseExecutionQueue.h"
#include "Wwise/WwiseFileState.h"

class WWISEFILEHANDLER_API FWwiseFileHandlerBase : protected IWwiseStreamingManagerHooks
{
protected:
	FWwiseFileHandlerBase();

	AKRESULT OpenStreaming(AkFileDesc& OutFileDesc, uint32 InShortId) override;
	void CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState) override;

	using FCreateStateFunction = TUniqueFunction<FWwiseFileStateSharedPtr()>;
	using FIncrementStateCallback = TUniqueFunction<void(const FWwiseFileStateSharedPtr&, bool bResult)>;
	using FDecrementStateCallback = TUniqueFunction<void()>;
	void IncrementFileStateUseAsync(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback);
	void DecrementFileStateUseAsync(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback);

	virtual void IncrementFileStateUse(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback);
	virtual void DecrementFileStateUse(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback);
	virtual void OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin);

	virtual const TCHAR* GetManagingTypeName() const { return TEXT("UNKNOWN"); }

	FWwiseExecutionQueue FileHandlerExecutionQueue;

	TMap<uint32, FWwiseFileStateSharedPtr> FileStatesById;
};
