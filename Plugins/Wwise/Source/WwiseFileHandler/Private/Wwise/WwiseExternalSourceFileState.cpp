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

#include "Wwise/WwiseExternalSourceFileState.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseExternalSourceFileState::FWwiseExternalSourceFileState(uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	AkExternalSourceInfo(),
	MediaId(InMediaId),
	MediaPathName(InMediaPathName),
	RootPath(InRootPath),
	PlayCount(0)
{
	idCodec = InCodecId;
	INC_DWORD_STAT(STAT_WwiseFileHandlerKnownExternalSourceMedia);
}

FWwiseExternalSourceFileState::~FWwiseExternalSourceFileState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerKnownExternalSourceMedia);
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


bool FWwiseExternalSourceFileState::DecrementPlayCount()
{
	const auto NewPlayCount = PlayCount.DecrementExchange() - 1;
	if (PlayCount < 0)
	{
		PlayCount.Store(0);
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseExternalSourceFileState: Play count went below zero for media %" PRIu32 " (%s)"),
		 MediaId, *MediaPathName.ToString());
	}
	return NewPlayCount == 0;
}

FWwiseInMemoryExternalSourceFileState::FWwiseInMemoryExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory, 
		uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMediaId, InMediaPathName, InRootPath, InCodecId),
	MemoryAlignment(InMemoryAlignment),
	bDeviceMemory(bInDeviceMemory),
	Ptr(nullptr),
	MappedHandle(nullptr),
	MappedRegion(nullptr)
{
#if WITH_EDITOR
	 if (bDeviceMemory)
	 {
	 	UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseExternalSourceFileState: Loading External Source Media with DeviceMemory=true while in in editor. Expect to see \"No Device Memory\" errors in the log."));
	 }
#endif
}

void FWwiseInMemoryExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(uiMemorySize || pInMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): Seems to be already opened."), MediaId, *MediaPathName.ToString());
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath.ToString() / MediaPathName.ToString();
	int64 FileSize = 0;
	if (LIKELY(GetFileToPtr(const_cast<const uint8*&>(reinterpret_cast<uint8*&>(pInMemory)), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Loading In-Memory Media."), MediaId, *MediaPathName.ToString());
		uiMemorySize = FileSize;
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
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Closing In-Memory Media."), MediaId, *MediaPathName.ToString());
	DeallocateMemory(const_cast<const uint8*&>(reinterpret_cast<uint8*&>(pInMemory)), uiMemorySize, bDeviceMemory, MemoryAlignment, true);
	pInMemory = nullptr;
	uiMemorySize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamedExternalSourceFileState::FWwiseStreamedExternalSourceFileState(uint32 InPrefetchSize, uint32 InStreamingGranularity,
	uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMediaId, InMediaPathName, InRootPath, InCodecId),
	PrefetchSize(InPrefetchSize),
	StreamingGranularity(InStreamingGranularity),
	StreamedFile(nullptr)
{
	idFile = InMediaId;
}

void FWwiseStreamedExternalSourceFileState::CloseStreaming()
{
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Closing streaming for external source media %" PRIu32 " (%s) without an ExternalSourceManager."), MediaId, *MediaPathName.ToString());
		return;
	}
	ExternalSourceManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

uint32 FWwiseStreamedExternalSourceFileState::GetPrefetchSize() const
{
	return (static_cast<uint32>(PrefetchSize) / StreamingGranularity + 1) * StreamingGranularity;
}

bool FWwiseStreamedExternalSourceFileState::CanOpenFile() const
{
#if WITH_EDITOR
	return State == EState::Closed && StreamingCount > 0;		// We need to wait for actual usage so we can regenerate SoundBanks.
#else
	return FWwiseFileState::CanOpenFile();
#endif
}

void FWwiseStreamedExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): Stream seems to be already opened."), MediaId, *MediaPathName.ToString());
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath.ToString() / MediaPathName.ToString();

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Loading Streaming Media."), MediaId, *MediaPathName.ToString());
	if (UNLIKELY(!OpenStreamedFile(StreamedFile, FullPathName, GetPrefetchSize())))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 ": Failed to load Streaming Media (%s)."), MediaId, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}
	iFileSize = StreamedFile->Size;

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
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("External Source %" PRIu32 " (%s): Closing Streaming Media."), MediaId, *MediaPathName.ToString());

	delete StreamedFile;
	StreamedFile = nullptr;
	iFileSize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

bool FWwiseStreamedExternalSourceFileState::CanProcessFileOp() const
{
	if (UNLIKELY(State != EState::Loaded))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source %" PRIu32 " (%s): IO Hook asked for a file operation, but state is not ready."), MediaId, *MediaPathName.ToString());
		return false;
	}
	return true;
}

AKRESULT FWwiseStreamedExternalSourceFileState::ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("External Source: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *MediaPathName.ToString());

	StreamedFile->ReadAsync(GetManagingTypeName(), MediaId, *MediaPathName.ToString(), InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));
	return AK_Success;
}
