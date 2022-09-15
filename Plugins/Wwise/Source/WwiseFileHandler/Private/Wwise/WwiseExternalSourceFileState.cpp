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

#include "Wwise/WwiseExternalSourceFileState.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseExternalSourceFileState::FWwiseExternalSourceFileState(uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId) :
	AkExternalSourceInfo(),
	MediaId(InMediaId),
	MediaPathName(InMediaPathName),
	RootPath(InRootPath),
	PlayCount(0)
{
	idCodec = InCodecId;
}

bool FWwiseExternalSourceFileState::GetExternalSourceInfo(AkExternalSourceInfo& OutInfo)
{
	OutInfo = static_cast<AkExternalSourceInfo>(*this);
	return szFile != nullptr || pInMemory != nullptr || idFile != 0;
}

void FWwiseExternalSourceFileState::IncrementPlayCount()
{
	++PlayCount;
}


void FWwiseExternalSourceFileState::DecrementPlayCount()
{
	PlayCount = --PlayCount; 
	if (PlayCount < 0)
	{
		PlayCount = 0;
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseExternalSourceFileState: Play count went below zero for media %" PRIu32 " (%s)"),
		 MediaId, *MediaPathName);
	}
}

FWwiseInMemoryExternalSourceFileState::FWwiseInMemoryExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory, 
		uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMediaId, InMediaPathName, InRootPath, InCodecId),
	MemoryAlignment(InMemoryAlignment),
	bDeviceMemory(bInDeviceMemory),
	Ptr(nullptr),
	MappedHandle(nullptr),
	MappedRegion(nullptr)
{
}

void FWwiseInMemoryExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(uiMemorySize || pInMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): Seems to be already opened."), MediaId, *MediaPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName;
	int64 FileSize = 0;
	if (LIKELY(GetFileToPtr(const_cast<const uint8*&>(reinterpret_cast<uint8*&>(pInMemory)), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Loading In-Memory Media."), MediaId, *MediaPathName);
		uiMemorySize = FileSize;
		INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedExternalSourceMedia);
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 ": Failed to open In-Memory Media (%s)."), MediaId, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryExternalSourceFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	LoadInSoundEngineSucceeded(MoveTemp(InCallback));
}

void FWwiseInMemoryExternalSourceFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

void FWwiseInMemoryExternalSourceFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Closing In-Memory Media."), MediaId, *MediaPathName);
	DeallocateMemory(const_cast<const uint8*&>(reinterpret_cast<uint8*&>(pInMemory)), uiMemorySize, bDeviceMemory, MemoryAlignment, true);
	pInMemory = nullptr;
	uiMemorySize = 0;
	DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedExternalSourceMedia);
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamedExternalSourceFileState::FWwiseStreamedExternalSourceFileState(uint32 InPrefetchSize, uint32 InStreamingGranularity,
	uint32 InMediaId, const FString& InMediaPathName, const FString& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMediaId, InMediaPathName, InRootPath, InCodecId),
	PrefetchSize(InPrefetchSize),
	StreamingGranularity(InStreamingGranularity),
	Archive(nullptr)
{
	idFile = InMediaId;
}

void FWwiseStreamedExternalSourceFileState::CloseStreaming()
{
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Closing streaming for external source media %" PRIu32 " (%s) without an ExternalSourceManager."), MediaId, *MediaPathName);
		return;
	}
	ExternalSourceManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

uint32 FWwiseStreamedExternalSourceFileState::GetPrefetchSize() const
{
	if (UNLIKELY(!Archive))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): Trying to get Prefetch Size without Archive"), MediaId, *MediaPathName);
		return 0;
	}
	auto Result = (static_cast<uint32>(PrefetchSize) / StreamingGranularity + 1) * StreamingGranularity;
	const auto TotalSize = Archive->TotalSize();
	if (TotalSize < UINT32_MAX && static_cast<uint32>(TotalSize) < Result)
	{
		Result = TotalSize;
	}
	return Result;
}

void FWwiseStreamedExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(iFileSize != 0 || Archive))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): Seems to be already opened."), MediaId, *MediaPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName;

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Loading Streaming Media."), MediaId, *MediaPathName);
	if (UNLIKELY(!GetFileArchive(Archive, FullPathName)))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 ": Failed to load Streaming Media (%s)."), MediaId, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}
	iFileSize = Archive->TotalSize();

	if (PrefetchSize > 0)
	{
		const auto CalculatedPrefetchSize = GetPrefetchSize();
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("External Source %" PRIu32 " (%s): Prefetching %" PRIu32 " bytes (Size: %" PRIi32 ", Granularity: %" PRIu32 ")."), MediaId, *MediaPathName,
			CalculatedPrefetchSize, PrefetchSize, StreamingGranularity);

		Archive->Precache(0, CalculatedPrefetchSize);
	}

	INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedExternalSourceMedia);
	OpenFileSucceeded(MoveTemp(InCallback));
}

void FWwiseStreamedExternalSourceFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	LoadInSoundEngineSucceeded(MoveTemp(InCallback));
}

void FWwiseStreamedExternalSourceFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

bool FWwiseStreamedExternalSourceFileState::CanCloseFile() const
{
#if WITH_EDITOR
	return State == EState::Opened && StreamingCount == 0;		// We need to quickly release streamed files so we can regenerate SoundBanks.
#else
	return FWwiseFileState::CanCloseFile();
#endif
}

void FWwiseStreamedExternalSourceFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Closing Streaming Media."), MediaId, *MediaPathName);
	auto* ArchiveCopy = Archive;
	Archive = nullptr;
	iFileSize = 0;
	AsyncClose(ArchiveCopy, [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedExternalSourceMedia);
		CloseFileDone(MoveTemp(InCallback));
	});
}

AKRESULT FWwiseStreamedExternalSourceFileState::ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("External Source: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *MediaPathName);

	AsyncRead(*Archive, GetManagingTypeName(), MediaId, *MediaPathName, InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));
	return AK_Success;
}
