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

#include "Wwise/WwiseFileStateTools.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include "Async/MappedFileHandle.h"
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
		UE_LOG(LogWwiseFileHandler, Fatal, TEXT("No Device Memory, but trying to allocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
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
		UE_LOG(LogWwiseFileHandler, Fatal, TEXT("No Device Memory, but trying to deallocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
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
	return GetMemoryMapped(OutMappedHandle, OutSize, InFilePathname, InMemoryAlignment)
		&& GetMemoryMappedRegion(OutMappedRegion, *OutMappedHandle);
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
		UE_LOG(LogWwiseFileHandler, Error, TEXT("MapRegion failed"));
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

	FArchive* Reader;
	if (!GetFileArchive(Reader, InFilePathname))
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

bool FWwiseFileStateTools::GetFileArchive(FArchive*& OutArchive, const FString& InFilePathname)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Creating file reader for %s"), *InFilePathname);

	FArchive* Reader = IFileManager::Get().CreateFileReader(*InFilePathname, 0);
	if (UNLIKELY(!Reader))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("CreateFileReader %s failed"), *InFilePathname);
		return false;
	}
	OutArchive = Reader;
	return true;
}

bool FWwiseFileStateTools::Read(FArchive& InArchive, uint8* OutBuffer, int64 InPosition, uint32 InSize)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseFileHandlerFileOperationLatency);

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("[Read] File %s: %" PRIu32 " bytes @ %" PRIi64), *InArchive.GetArchiveName(), InSize, InPosition);
	InArchive.Seek(InPosition);
	InArchive.Serialize(OutBuffer, InSize);
	const bool bError = InArchive.IsError();

	UE_CLOG(bError, LogWwiseFileHandler, Verbose, TEXT("[Read] Failed reading file %s: %" PRIu32 " bytes @ %" PRIi64), *InArchive.GetArchiveName(), InSize, InPosition);

	if (!bError)
	{
		ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerTotalStreamedMB, static_cast<float>(InSize) / 1024 / 1024);
		ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerStreamingKB, static_cast<float>(InSize) / 1024);
	}
	return !bError;
}

void FWwiseFileStateTools::AsyncRead(FArchive& InArchive, FWwiseExecutionQueue::FTimedCallback&& InCallback,
                                     uint8* OutBuffer, int64 InPosition, uint32 InSize,
                                     FTimespan InDeadline, AkPriority InPriority)
{
	FileStateExecutionQueue.Async(MoveTemp(InCallback), [&InArchive, OutBuffer, InPosition, InSize, InPriority](FTimespan InDeadline)
	{
		return Read(InArchive, OutBuffer, InPosition, InSize) ? FWwiseExecutionQueue::ETimedResult::Success : FWwiseExecutionQueue::ETimedResult::Failure;
	}, InDeadline);
}

void FWwiseFileStateTools::AsyncRead(FArchive& InArchive, const TCHAR* InAssetType, uint32 InAssetId, const TCHAR* InAssetName, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("AsyncRead %s %" PRIu32 " (%s)"), InAssetType, InAssetId, InAssetName);
	
	const auto StartTime = FWwiseExecutionQueue::Now();
	const auto Deadline = InHeuristics.fDeadline == .0f ? FWwiseExecutionQueue::NoTimeLimit() : StartTime + FTimespan::FromMilliseconds(InHeuristics.fDeadline);
	
	AsyncRead(InArchive, [InAssetType, InAssetId, InAssetName, InHeuristics, OutTransferInfo, Deadline, InFileOpDoneCallback = MoveTemp(InFileOpDoneCallback), StartTime](FWwiseExecutionQueue::ETimedResult InResult) mutable
	{
		const auto EndTime = FWwiseExecutionQueue::Now();
		const auto TimeSpan = static_cast<int>((EndTime - StartTime).GetTotalMicroseconds());

		AKRESULT Result;
		if (LIKELY(InResult == FWwiseExecutionQueue::ETimedResult::Success))
		{
			Result = AK_Success;

			if (Deadline == FWwiseExecutionQueue::NoTimeLimit())
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("AsyncRead %s %" PRIu32 " in %dus"), InAssetType, InAssetId, TimeSpan);
			}
			else
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("AsyncRead %s %" PRIu32 " in %dus"), InAssetType, InAssetId, TimeSpan);
			}
		}
		else if (InResult == FWwiseExecutionQueue::ETimedResult::Timeout)
		{
			Result = AK_Success;
			const auto DeadlineUS = static_cast<int>((Deadline - StartTime).GetTotalMicroseconds());
			UE_LOG(LogWwiseFileHandler, Error, TEXT("AsyncRead: Timed out reading %s %" PRIu32 " (%s) in %dus: Timed out at %dus"), InAssetName, InAssetId, InAssetName, DeadlineUS, TimeSpan);
		}
		else // if (InResult == FWwiseExecutionQueue::ETimedResult::Failure)
		{
			Result = AK_UnknownFileError;
			UE_LOG(LogWwiseFileHandler, Error, TEXT("AsyncRead: Failed reading %s %" PRIu32 " (%s) (%dus)"), InAssetName, InAssetId, InAssetName, TimeSpan);
		}

		InFileOpDoneCallback(&OutTransferInfo, Result);
	}, static_cast<uint8*>(OutTransferInfo.pBuffer), OutTransferInfo.uFilePosition, OutTransferInfo.uRequestedSize, Deadline, InHeuristics.priority);
}

void FWwiseFileStateTools::AsyncClose(FArchive* InArchive, FWwiseExecutionQueue::FBasicFunction&& InCallback)
{
	if (LIKELY(InArchive)) FileStateExecutionQueue.Async([InArchive, InCallback = MoveTemp(InCallback)] {
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Closing archive for file %s."), *InArchive->GetArchiveName());
		if (LIKELY(InArchive->Close()))
		{
			delete InArchive;
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("Unable to close Archive for file %s. Leaking."), *InArchive->GetArchiveName());
		}
		InCallback();
	});
}
