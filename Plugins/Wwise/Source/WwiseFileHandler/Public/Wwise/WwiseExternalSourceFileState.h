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

#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

class WWISEFILEHANDLER_API FWwiseExternalSourceFileState : public FWwiseFileState, public AkExternalSourceInfo
{
public:
	const uint32 MediaId;
	const FString MediaPathName;
	const FString RootPath;

	int PlayCount;

protected:
	FWwiseExternalSourceFileState(uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId);

public:
	bool CanDelete() const override { return FWwiseFileState::CanDelete() && PlayCount == 0; }

	virtual bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo);
	void IncrementPlayCount();
	void DecrementPlayCount();

protected:
	bool CanUnloadFromSoundEngine() const override { return FWwiseFileState::CanUnloadFromSoundEngine() && PlayCount == 0; }

	const TCHAR* GetManagingTypeName() const override final { return TEXT("External Source"); }
	uint32 GetShortId() const override final { return MediaId; }
};

class WWISEFILEHANDLER_API FWwiseInMemoryExternalSourceFileState : public FWwiseExternalSourceFileState
{
public:
	const uint32 MemoryAlignment;
	const bool bDeviceMemory;

	const uint8* Ptr;
	IMappedFileHandle* MappedHandle;
	IMappedFileRegion* MappedRegion;

	FWwiseInMemoryExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory, 
		uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId);
	~FWwiseInMemoryExternalSourceFileState() override { FileStateExecutionQueue.Stop(); }

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	void CloseFile(FCloseFileCallback&& InCallback) override;
};

class WWISEFILEHANDLER_API FWwiseStreamedExternalSourceFileState : public FWwiseExternalSourceFileState, public FWwiseStreamableFileStateInfo
{
public:
	const uint32 PrefetchSize;
	const uint32 StreamingGranularity;

	FArchive* Archive;

	FWwiseStreamedExternalSourceFileState(uint32 InPrefetchSize, uint32 InStreamingGranularity,
		uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId);
	~FWwiseStreamedExternalSourceFileState() override { FileStateExecutionQueue.Stop(); }

	void CloseStreaming() override;
	FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() override { return this; }
	const FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() const override { return this; }

	uint32 GetPrefetchSize() const;

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	bool CanCloseFile() const override;
	void CloseFile(FCloseFileCallback&& InCallback) override;

	AKRESULT ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback) override;
};
