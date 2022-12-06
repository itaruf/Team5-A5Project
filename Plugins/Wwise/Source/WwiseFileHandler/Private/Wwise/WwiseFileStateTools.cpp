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

#include "Wwise/WwiseFileStateTools.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include "AkUEFeatures.h"

#include "Misc/Paths.h"
#include "Async/MappedFileHandle.h"
#include "Async/AsyncFileHandle.h"
#include "HAL/FileManager.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#include <inttypes.h>

uint8* FWwiseFileStateTools::AllocateMemory(int64 InMemorySize, bool bInDeviceMemory, int32 InMemoryAlignment,
	bool bInEnforceMemoryRequirements)
{
	uint8* Result = nullptr;

	if (bInDeviceMemory && bInEnforceMemoryRequirements)
	{
#if AK_SUPPORT_DEVICE_MEMORY
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Allocating %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		Result = static_cast<uint8*>(AKPLATFORM::AllocDevice((size_t)InMemorySize, nullptr));
		if (Result)
		{
			INC_MEMORY_STAT_BY(STAT_WwiseFileHandlerDeviceMemoryAllocated, InMemorySize);
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not allocate %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		}
#else
		UE_LOG(LogWwiseFileHandler, Error, TEXT("No Device Memory, but trying to allocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
		return AllocateMemory(InMemorySize, false, InMemoryAlignment, bInEnforceMemoryRequirements);
#endif
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Allocating %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
		Result = static_cast<uint8*>(FMemory::Malloc(InMemorySize, bInEnforceMemoryRequirements ? InMemoryAlignment : 0));
		if (Result)
		{
			INC_MEMORY_STAT_BY(STAT_WwiseFileHandlerMemoryAllocated, InMemorySize);
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not allocate %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
		}
	}
	return Result;
}

void FWwiseFileStateTools::DeallocateMemory(const uint8* InMemoryPtr, int64 InMemorySize, bool bInDeviceMemory,
	int32 InMemoryAlignment, bool bInEnforceMemoryRequirements)
{
	if (!InMemoryPtr)
	{
		return;
	}

	if (bInDeviceMemory && bInEnforceMemoryRequirements)
	{
#if AK_SUPPORT_DEVICE_MEMORY
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Deallocating %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		DEC_MEMORY_STAT_BY(STAT_WwiseFileHandlerDeviceMemoryAllocated, InMemorySize);
		AKPLATFORM::FreeDevice((void*)InMemoryPtr, InMemorySize, 0, true);
#else
		UE_LOG(LogWwiseFileHandler, Error, TEXT("No Device Memory, but trying to deallocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
		return DeallocateMemory(InMemoryPtr, InMemorySize, false, InMemoryAlignment, bInEnforceMemoryRequirements);

#endif
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Deallocating %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
		DEC_MEMORY_STAT_BY(STAT_WwiseFileHandlerMemoryAllocated, InMemorySize);
		FMemory::Free(const_cast<uint8*>(InMemoryPtr));
	}
}

bool FWwiseFileStateTools::GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, IMappedFileRegion*& OutMappedRegion,
	int64& OutSize, const FString& InFilePathname, int32 InMemoryAlignment)
{
	if (!GetMemoryMapped(OutMappedHandle, OutSize, InFilePathname, InMemoryAlignment))
	{
		return false;
	}
	if (UNLIKELY(!GetMemoryMappedRegion(OutMappedRegion, *OutMappedHandle)))
	{
		UnmapHandle(*OutMappedHandle);
		OutMappedHandle = nullptr;
		return false;
	}
	return true;
}

bool FWwiseFileStateTools::GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, int64& OutSize,
	const FString& InFilePathname, int32 InMemoryAlignment)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Memory mapping %s"), *InFilePathname);
	auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	auto* Handle = PlatformFile.OpenMapped(*InFilePathname);
	if (UNLIKELY(!Handle))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OpenMapped %s failed"), *InFilePathname);
		return false;
	}

	OutMappedHandle = Handle;
	OutSize = Handle->GetFileSize();
	return true;
}

bool FWwiseFileStateTools::GetMemoryMappedRegion(IMappedFileRegion*& OutMappedRegion,
	IMappedFileHandle& InMappedHandle)
{
	auto* Region = InMappedHandle.MapRegion(0, MAX_int64, true);
	if (UNLIKELY(!Region))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("MapRegion failed"));
		return false;
	}

	INC_MEMORY_STAT_BY(STAT_WwiseFileHandlerMemoryMapped, Region->GetMappedSize());
	OutMappedRegion = Region;
	return true;
}

void FWwiseFileStateTools::UnmapRegion(IMappedFileRegion& InMappedRegion)
{
	DEC_MEMORY_STAT_BY(STAT_WwiseFileHandlerMemoryMapped, InMappedRegion.GetMappedSize());
	delete &InMappedRegion;
}

void FWwiseFileStateTools::UnmapHandle(IMappedFileHandle& InMappedHandle)
{
	delete &InMappedHandle;
}

bool FWwiseFileStateTools::GetFileToPtr(const uint8*& OutPtr, int64& OutSize, const FString& InFilePathname,
	bool bInDeviceMemory, int32 InMemoryAlignment, bool bInEnforceMemoryRequirements)
{
	FScopedLoadingState ScopedLoadingState(*InFilePathname);

	FArchive* Reader = IFileManager::Get().CreateFileReader(*InFilePathname, 0);
	if (!Reader)
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Could not get File Archive for %s"), *InFilePathname);
		return false;
	}

	const int64 Size = Reader->TotalSize();
	if (UNLIKELY(!Size))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Empty file %s"), *InFilePathname);
		delete Reader;
		return false;
	}

	if (UNLIKELY((InMemoryAlignment & (InMemoryAlignment - 1)) != 0))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("Invalid non-2^n Memory Alignment (%" PRIi32 ") while getting file %s. Resetting to 0."), InMemoryAlignment, *InFilePathname);
		InMemoryAlignment = 0;
	}

	uint8* Ptr = AllocateMemory(Size, bInDeviceMemory, InMemoryAlignment, bInEnforceMemoryRequirements);
	if (UNLIKELY(!Ptr))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Could not Allocate memory for %s"), *InFilePathname);
		delete Reader;
		return false;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Getting a copy of full file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);

	Reader->Serialize(Ptr, Size);
	const bool Result = Reader->Close();

	delete Reader;
	if (!Result)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Deserialization failed for file %s"), *InFilePathname);
		DeallocateMemory(Ptr, Size, bInDeviceMemory, InMemoryAlignment, bInEnforceMemoryRequirements);
		return false;
	}

	OutPtr = Ptr;
	OutSize = Size;
	return true;
}

bool FWwiseFileStateTools::OpenStreamedFile(FWwiseStreamedFile*& OutFile, const FString& InFilePathname, int64 InPreloadSize)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseFileHandlerFileOperationLatency);

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OpenStreamedFile: Creating file reader for %s"), *InFilePathname);

	IAsyncReadFileHandle* FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenAsyncRead(*InFilePathname);
	if (UNLIKELY(!FileHandle))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OpenStreamedFile: OpenAsyncRead %s failed"), *InFilePathname);
		OutFile = nullptr;
		return false;
	}

	// This makes the Open operation synchronous.
	auto* SizeRequest = FileHandle->SizeRequest();
	if (UNLIKELY(!SizeRequest || !SizeRequest->WaitCompletion()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OpenStreamedFile: SizeRequest %s failed"), *InFilePathname);
		delete SizeRequest;
		delete FileHandle;
		OutFile = nullptr;
		return false;
	}
	const auto Size = SizeRequest->GetSizeResults();
	delete SizeRequest;
	if (UNLIKELY(Size <= 0))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("OpenStreamedFile: File %s not found or empty."), *InFilePathname);
		delete FileHandle;
		OutFile = nullptr;
		return false;
	}

	const auto Handle = IFileCacheHandle::CreateFileCacheHandle(FileHandle);
	if (UNLIKELY(!Handle))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OpenStreamedFile: CreateFileCacheHandle %s failed"), *InFilePathname);
		delete FileHandle;
		OutFile = nullptr;
		return false;
	}
	
	const auto FileName = FPaths::GetCleanFilename(InFilePathname);
	InPreloadSize = FMath::Min(InPreloadSize, Size);
	if (InPreloadSize > 0)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OpenStreamedFile: Preloading %" PRIi64 " bytes for %s"), InPreloadSize, *FileName);
		ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerPreloadingKB, static_cast<float>(InPreloadSize) / 1024);
		const FFileCachePreloadEntry PreloadEntry(0, InPreloadSize);
		const auto Task = Handle->PreloadData(&PreloadEntry, 1, EAsyncIOPriorityAndFlags::AIOP_Low);
		if (!Task.IsValid())
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OpenStreamedFile: PreloadData failed"));
			delete FileHandle;
			OutFile = nullptr;
			return false;
		}
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
	}

	OutFile = new FWwiseStreamedFile(Handle, FileName, Size);
	return true;
}

FWwiseStreamedFile::FWwiseStreamedFile(IFileCacheHandle* InHandle, const FString& InFileName, int64 InSize) :
	Handle(InHandle),
	FileName(InFileName),
	Size(InSize)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Opened streamed file %s."), *FileName);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
}

FWwiseStreamedFile::~FWwiseStreamedFile()
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Closing streamed file %s."), *FileName);
	ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
}

bool FWwiseStreamedFile::Read(uint8* OutBuffer, int64 InPosition, uint32 InSize) const
{
	bool bResult = false;
	FEventRef Event;
	ReadAsync([&Event, &bResult](FWwiseExecutionQueue::ETimedResult InResult)
	{
		bResult = InResult == FWwiseExecutionQueue::ETimedResult::Success;
		Event->Trigger();
	}, OutBuffer, InPosition, InSize);
	Event->Wait();
	return bResult;
}

void FWwiseStreamedFile::ReadAsync(FWwiseExecutionQueue::FTimedCallback&& InCallback, uint8* OutBuffer,
	int64 InPosition, uint32 InSize, FTimespan InDeadline, AkPriority InPriority) const
{
	FWwiseAsyncCycleCounter Stat(GET_STATID(STAT_WwiseFileHandlerFileOperationLatency));

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("[Read] Stream file %s: %" PRIu32 " bytes @ %" PRIi64), *FileName, InSize, InPosition);

	EAsyncIOPriorityAndFlags Priority;
	if (LIKELY(InPriority == AK_DEFAULT_PRIORITY))
	{
		Priority = AIOP_Normal;
	}
	else if (InPriority == AK_MIN_PRIORITY)
	{
		Priority = AIOP_MIN;
	}
	else if (InPriority == AK_MAX_PRIORITY)
	{
		Priority = AIOP_MAX;
	}
	else if (InPriority < AK_DEFAULT_PRIORITY)
	{
		Priority = AIOP_Low;
	}
	else // if (InPriority > AK_DEFAULT_PRIORITY)
	{
		Priority = AIOP_High;
	}

	FGraphEventArray CompletionTasks;
	auto ReadData = Handle->ReadData(CompletionTasks, InPosition, InSize, Priority);
	if (UNLIKELY(!ReadData))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("[Read] Failed streaming file %s: %" PRIu32 " bytes @ %" PRIi64 ". No lanes available."), *FileName, InSize, InPosition);
		InCallback(FWwiseExecutionQueue::ETimedResult::Failure);
	}
	else
	{
		Async(EAsyncExecution::TaskGraph, [this, ReadData = MoveTemp(ReadData), CompletionTasks = MoveTemp(CompletionTasks), MoveTemp(Stat), OutBuffer, InSize, InPosition, InCallback = MoveTemp(InCallback)]
		{
			FTaskGraphInterface::Get().WaitUntilTasksComplete(CompletionTasks);
			if (UNLIKELY(ReadData->GetSize() < InSize))
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("[Read] Failed streaming file %s: %" PRIu32 " bytes @ %" PRIi64), *FileName, InSize, InPosition);
				InCallback(FWwiseExecutionQueue::ETimedResult::Failure);
			}
			else
			{
				ReadData->CopyTo(OutBuffer, 0, InSize);
				ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerTotalStreamedMB, static_cast<float>(InSize) / 1024 / 1024);
				ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerStreamingKB, static_cast<float>(InSize) / 1024);

				InCallback(FWwiseExecutionQueue::ETimedResult::Success);
			}
		});
	}}

void FWwiseStreamedFile::ReadAsync(const TCHAR* InAssetType, uint32 InAssetId, const TCHAR* InAssetName,
	const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo,
	TFileOpDoneCallback&& InFileOpDoneCallback) const
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ReadStreamedFileAsync Request for %s %" PRIu32 " (%s)"), InAssetType, InAssetId, InAssetName);
	
	const auto StartTime = FWwiseExecutionQueue::Now();
	const auto Deadline = InHeuristics.fDeadline == .0f ? FWwiseExecutionQueue::NoTimeLimit() : StartTime + FTimespan::FromMilliseconds(InHeuristics.fDeadline);
	
	ReadAsync([InAssetType, InAssetId, InAssetName, InHeuristics, &OutTransferInfo, Deadline, InFileOpDoneCallback = MoveTemp(InFileOpDoneCallback), StartTime](FWwiseExecutionQueue::ETimedResult InResult) mutable
	{
		const auto EndTime = FWwiseExecutionQueue::Now();
		const auto TimeSpan = static_cast<int>((EndTime - StartTime).GetTotalMicroseconds());

		AKRESULT Result;
		if (LIKELY(InResult == FWwiseExecutionQueue::ETimedResult::Success))
		{
			Result = AK_Success;

			if (Deadline == FWwiseExecutionQueue::NoTimeLimit())
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ReadStreamedFileAsync %s %" PRIu32 " in %dus"), InAssetType, InAssetId, TimeSpan);
			}
			else
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ReadStreamedFileAsync %s %" PRIu32 " in %dus"), InAssetType, InAssetId, TimeSpan);
			}
		}
		else if (InResult == FWwiseExecutionQueue::ETimedResult::Timeout)
		{
			Result = AK_Success;
			const auto DeadlineUS = static_cast<int>((Deadline - StartTime).GetTotalMicroseconds());
			UE_LOG(LogWwiseFileHandler, Error, TEXT("ReadStreamedFileAsync: Timed out reading %s %" PRIu32 " (%s) in %dus: Timed out at %dus"), InAssetName, InAssetId, InAssetName, DeadlineUS, TimeSpan);
		}
		else // if (InResult == FWwiseExecutionQueue::ETimedResult::Failure)
		{
			Result = AK_UnknownFileError;
			UE_LOG(LogWwiseFileHandler, Error, TEXT("ReadStreamedFileAsync: Failed reading %s %" PRIu32 " (%s) (%dus)"), InAssetName, InAssetId, InAssetName, TimeSpan);
		}

		InFileOpDoneCallback(&OutTransferInfo, Result);
	}, static_cast<uint8*>(OutTransferInfo.pBuffer), OutTransferInfo.uFilePosition, OutTransferInfo.uRequestedSize, Deadline, InHeuristics.priority);
}
