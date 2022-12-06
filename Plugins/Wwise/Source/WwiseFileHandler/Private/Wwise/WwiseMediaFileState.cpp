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

#include "Wwise/WwiseMediaFileState.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"
#include "AkUnrealHelper.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseMediaFileState::FWwiseMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath) :
	FWwiseMediaCookedData(InCookedData),
	RootPath(InRootPath)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerKnownMedia);
}

FWwiseMediaFileState::~FWwiseMediaFileState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerKnownMedia);
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
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Seems to be already opened."), MediaId, *DebugName.ToString());
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName.ToString();

	int64 FileSize = 0;
	if (LIKELY(GetFileToPtr(const_cast<const uint8*&>(pMediaMemory), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Loading In-Memory Media."), MediaId, *DebugName.ToString());
		uMediaSize = FileSize;
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to open In-Memory Media (%s)."), MediaId, *DebugName.ToString(), *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(!uMediaSize || !pMediaMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): No data, but supposed to be loaded."), MediaId, *DebugName.ToString());
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed loading media %" PRIu32 " (%s) without a SoundEngine."), MediaId, *DebugName.ToString());
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
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to load Media: %d (%s)."), MediaId, *DebugName.ToString(), SetMediaResult, AkUnrealHelper::GetResultString(SetMediaResult));
		LoadInSoundEngineFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s) without a SoundEngine."), MediaId, *DebugName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	const auto Result = SoundEngine->TryUnsetMedia(this, 1, nullptr);
	if (UNLIKELY(Result == AK_ResourceInUse))
	{
		UnloadFromSoundEngineDefer(MoveTemp(InCallback));
	}
	else
	{
		UE_CLOG(Result != AK_Success, LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): TryUnsetMedia failed: %d (%s)"), MediaId, *DebugName.ToString(), Result, AkUnrealHelper::GetResultString(Result));
		DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		UnloadFromSoundEngineDone(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Closing In-Memory Media."), MediaId, *DebugName.ToString());
	DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
	pMediaMemory = nullptr;
	uMediaSize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamingMediaFileState::FWwiseStreamingMediaFileState(const FWwiseMediaCookedData& InCookedData,
	const FString& InRootPath, uint32 InStreamingGranularity) :
	FWwiseMediaFileState(InCookedData, InRootPath),
	StreamingGranularity(InStreamingGranularity),
	StreamedFile(nullptr)
{
}

uint32 FWwiseStreamingMediaFileState::GetPrefetchSize() const
{
	return (static_cast<uint32>(PrefetchSize) / StreamingGranularity + 1) * StreamingGranularity;
}

void FWwiseStreamingMediaFileState::CloseStreaming()
{
	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Closing streaming for media %" PRIu32 " (%s) without a MediaManager."), MediaId, *DebugName.ToString());
		return;
	}
	MediaManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

bool FWwiseStreamingMediaFileState::CanOpenFile() const
{
#if WITH_EDITOR
	return State == EState::Closed && StreamingCount > 0;		// We need to wait for actual usage so we can regenerate SoundBanks.
#else
	return FWwiseFileState::CanOpenFile();
#endif
}

void FWwiseStreamingMediaFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Stream seems to be already opened."), MediaId, *DebugName.ToString());
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / MediaPathName.ToString();

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Loading Streaming Media."), MediaId, *DebugName.ToString());
	if (UNLIKELY(!OpenStreamedFile(StreamedFile, FullPathName, GetPrefetchSize())))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): Failed to load Streaming Media (%s)."), MediaId, *DebugName.ToString(), *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}
	iFileSize = StreamedFile->Size;

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
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Media %" PRIu32 " (%s): Closing Streaming Media."), MediaId, *DebugName.ToString());
	delete StreamedFile;
	StreamedFile = nullptr;
	iFileSize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

bool FWwiseStreamingMediaFileState::CanProcessFileOp() const
{
	if (UNLIKELY(State != EState::Loaded))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Media %" PRIu32 " (%s): IO Hook asked for a file operation, but state is not ready."), MediaId, *DebugName.ToString());
		return false;
	}
	return true;
}

AKRESULT FWwiseStreamingMediaFileState::ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics,
                                                    AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamingMediaFileState: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *DebugName.ToString());

	StreamedFile->ReadAsync(GetManagingTypeName(), MediaId, *DebugName.ToString(), InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));

	return AK_Success;
}
