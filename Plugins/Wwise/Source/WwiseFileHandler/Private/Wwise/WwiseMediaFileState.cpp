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

#include "Wwise/WwiseMediaFileState.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/Stats/AsyncStats.h"
#include "AkUnrealHelper.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseMediaFileState::FWwiseMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath) :
	FWwiseMediaCookedData(InCookedData),
	RootPath(InRootPath)
{
}

FWwiseInMemoryMediaFileState::FWwiseInMemoryMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath) :
	FWwiseMediaFileState(InCookedData, InRootPath)
{
	pMediaMemory = nullptr;
	sourceID = MediaId;
	uMediaSize = 0;
}

void FWwiseInMemoryMediaFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(uMediaSize || pMediaMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Seems to be already opened."), MediaId, *DebugName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName;

	int64 FileSize = 0;
	if (LIKELY(GetFileToPtr(const_cast<const uint8*&>(pMediaMemory), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Loading In-Memory Media."), MediaId, *DebugName);
		uMediaSize = FileSize;
		INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedMedia);
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to open In-Memory Media (%s)."), MediaId, *DebugName, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(!uMediaSize || !pMediaMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): No data, but supposed to be loaded."), MediaId, *DebugName);
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed loading media %" PRIu32 " (%s) without a SoundEngine."), MediaId, *DebugName);
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	const auto SetMediaResult = SoundEngine->SetMedia(this, 1);
	if (LIKELY(SetMediaResult == AK_Success))
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		LoadInSoundEngineSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to load Media: %d (%s)."), MediaId, *DebugName, SetMediaResult, AkUnrealHelper::GetResultString(SetMediaResult));
		LoadInSoundEngineFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s) without a SoundEngine."), MediaId, *DebugName);
		return CloseFileDone(MoveTemp(InCallback));
	}

	const auto Result = SoundEngine->TryUnsetMedia(this, 1, nullptr);
	if (UNLIKELY(Result == AK_ResourceInUse))
	{
		UnloadFromSoundEngineDefer(MoveTemp(InCallback));
	}
	else
	{
		UE_CLOG(Result != AK_Success, LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): TryUnsetMedia failed: %d (%s)"), MediaId, *DebugName, Result, AkUnrealHelper::GetResultString(Result));
		DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		UnloadFromSoundEngineDone(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Closing In-Memory Media."), MediaId, *DebugName);
	DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
	pMediaMemory = nullptr;
	uMediaSize = 0;
	DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedMedia);
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamingMediaFileState::FWwiseStreamingMediaFileState(const FWwiseMediaCookedData& InCookedData,
	const FString& InRootPath, uint32 InStreamingGranularity) :
	FWwiseMediaFileState(InCookedData, InRootPath),
	StreamingGranularity(InStreamingGranularity),
	Archive(nullptr)
{
}

uint32 FWwiseStreamingMediaFileState::GetPrefetchSize() const
{
	auto Result = (static_cast<uint32>(PrefetchSize) / StreamingGranularity + 1) * StreamingGranularity;
	const auto TotalSize = Archive->TotalSize();
	if (TotalSize < UINT32_MAX && static_cast<uint32>(TotalSize) < Result)
	{
		Result = TotalSize;
	}
	return Result;
}

void FWwiseStreamingMediaFileState::CloseStreaming()
{
	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Closing streaming for media %" PRIu32 " (%s) without a MediaManager."), MediaId, *DebugName);
		return;
	}
	MediaManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

void FWwiseStreamingMediaFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(iFileSize != 0 || Archive))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Seems to be already opened."), MediaId, *DebugName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName;

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Loading Streaming Media."), MediaId, *DebugName);
	if (UNLIKELY(!GetFileArchive(Archive, FullPathName)))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to load Streaming Media (%s)."), MediaId, *DebugName, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}
	iFileSize = Archive->TotalSize();

	if (PrefetchSize > 0)
	{
		const auto CalculatedPrefetchSize = GetPrefetchSize();
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Media %" PRIu32 " (%s): Prefetching %" PRIu32 " bytes (Size: %" PRIi32 ", Granularity: %" PRIu32 ")."), MediaId, *DebugName,
			CalculatedPrefetchSize, PrefetchSize, StreamingGranularity);

		Archive->Precache(0, CalculatedPrefetchSize);
	}

	INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedMedia);
	OpenFileSucceeded(MoveTemp(InCallback));
}

void FWwiseStreamingMediaFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
	LoadInSoundEngineSucceeded(MoveTemp(InCallback));
}

void FWwiseStreamingMediaFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

bool FWwiseStreamingMediaFileState::CanCloseFile() const
{
#if WITH_EDITOR
	return State == EState::Opened && StreamingCount == 0;		// We need to quickly release streamed files so we can regenerate SoundBanks.
#else
	return FWwiseFileState::CanCloseFile();
#endif
}

void FWwiseStreamingMediaFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Closing Streaming Media."), MediaId, *DebugName);
	auto* ArchiveCopy = Archive;
	Archive = nullptr;
	iFileSize = 0;
	AsyncClose(ArchiveCopy, [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedMedia);
		CloseFileDone(MoveTemp(InCallback));
	});
}

AKRESULT FWwiseStreamingMediaFileState::ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics,
	AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamingMediaFileState: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *DebugName);

	// Load first bytes in order up until PrefetchSize is exhausted. Then, start AsyncRead.
	if (OutTransferInfo.uFilePosition + OutTransferInfo.uRequestedSize <= PrefetchSize)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamingMediaFileState Reading Media %" PRIu32 " (%s)"), MediaId, *DebugName);
		const bool bResult = Read(*Archive, static_cast<uint8*>(OutTransferInfo.pBuffer), OutTransferInfo.uFilePosition, OutTransferInfo.uRequestedSize);

		InFileOpDoneCallback(&OutTransferInfo, bResult ? AK_Success : AK_UnknownFileError);
	}
	else
	{
		if (UNLIKELY(PrefetchSize > 0))
		{
			PrefetchSize = 0;
		}

		AsyncRead(*Archive, GetManagingTypeName(), MediaId, *DebugName, InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));
	}

	return AK_Success;
}
