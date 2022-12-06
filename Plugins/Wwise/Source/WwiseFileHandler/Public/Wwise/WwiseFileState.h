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

#include "Wwise/WwiseDeferredQueue.h"
#include "Wwise/WwiseFileStateTools.h"

class FWwiseStreamableFileStateInfo;

enum class WWISEFILEHANDLER_API EWwiseFileStateOperationOrigin
{
	Loading,
	Streaming
};

class WWISEFILEHANDLER_API FWwiseFileState : protected FWwiseFileStateTools
{
public:
	~FWwiseFileState() override;

	template <typename RequestedType>
	RequestedType* GetStateAs()
	{
#if defined(WITH_RTTI) || defined(_CPPRTTI) || defined(__GXX_RTTI)
		auto* Result = dynamic_cast<RequestedType*>(this);
		checkf(Result, TEXT("Invalid Type Cast"));
#else
		auto* Result = static_cast<RequestedType*>(this);
#endif
		return Result;
	}

	FWwiseExecutionQueue FileStateExecutionQueue;
	
	int LoadCount;
	int StreamingCount;

	enum class WWISEFILEHANDLER_API EState
	{
		Closed,
		Opening,
		Opened,
		Loading,
		Loaded,
		Unloading,
		Closing,

		WillReload,			// While unloading, a task is waiting to load again
		CanReload,			// Equivalent to Opened, but won't Close
		WillReopen,			// While closing, a task is waiting to open again
		CanReopen			// Equivalent to Closed, but won't Delete
	};
	EState State;

	using FIncrementCountCallback = TUniqueFunction<void(bool bInResult)>;
	void IncrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);

	using FDecrementCountCallback = TUniqueFunction<void()>;
	using FDeleteFileStateFunction = TUniqueFunction<void(FDecrementCountCallback&& InCallback)>;
	void DecrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);

	virtual bool CanDelete() const;
	virtual const TCHAR* GetManagingTypeName() const { return TEXT("Invalid"); }
	virtual uint32 GetShortId() const { return 0; }

	virtual FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() { return nullptr; }
	virtual const FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo()  const { return nullptr; }
	bool IsStreamedState() const { return GetStreamableFileStateInfo() != nullptr; }

protected:
	FWwiseFileState();
	void Term();

	virtual void IncrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountOpen(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountLoad(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);

	virtual void DecrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountUnload(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountClose(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);

	using FOpenFileCallback = TUniqueFunction<void()>;
	using FLoadInSoundEngineCallback = TUniqueFunction<void()>;
	virtual void IncrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin);

	virtual bool CanOpenFile() const;
	virtual void OpenFile(FOpenFileCallback&& InCallback) { OpenFileFailed(MoveTemp(InCallback)); }
	void OpenFileSucceeded(FOpenFileCallback&& InCallback);
	void OpenFileFailed(FOpenFileCallback&& InCallback);

	virtual bool CanLoadInSoundEngine() const;
	virtual void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) { LoadInSoundEngineFailed(MoveTemp(InCallback)); }
	void LoadInSoundEngineSucceeded(FLoadInSoundEngineCallback&& InCallback);
	void LoadInSoundEngineFailed(FLoadInSoundEngineCallback&& InCallback);

	using FUnloadFromSoundEngineCallback = TUniqueFunction<void(EWwiseDeferredAsyncResult InResult)>;
	using FCloseFileCallback = TUniqueFunction<void(EWwiseDeferredAsyncResult InResult)>;
	virtual void DecrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin);

	virtual bool CanUnloadFromSoundEngine() const;
	virtual void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) { UnloadFromSoundEngineDone(MoveTemp(InCallback)); }
	void UnloadFromSoundEngineDone(FUnloadFromSoundEngineCallback&& InCallback);
	void UnloadFromSoundEngineToClosedFile(FUnloadFromSoundEngineCallback&& InCallback);
	void UnloadFromSoundEngineDefer(FUnloadFromSoundEngineCallback&& InCallback);

	virtual bool CanCloseFile() const;
	virtual void CloseFile(FCloseFileCallback&& InCallback) { CloseFileDone(MoveTemp(InCallback)); }
	void CloseFileDone(FCloseFileCallback&& InCallback);
	void CloseFileDefer(FCloseFileCallback&& InCallback);
};

using FWwiseFileStateSharedPtr = TSharedPtr<FWwiseFileState, ESPMode::ThreadSafe>;
