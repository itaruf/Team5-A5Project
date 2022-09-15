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

#include "Wwise/WwiseIOHookImpl.h"

#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseWriteFileState.h"
#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseStreamableFileHandler.h"

#include "Wwise/Stats/AsyncStats.h"

#include "AkUnrealHelper.h"

#include "Async/Async.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#include <inttypes.h>

FWwiseIOHookImpl::FWwiseIOHookImpl()
#ifndef AK_OPTIMIZED
:
	CurrentDeviceData(0),
	MaxDeviceData(0)
#endif
{
}

bool FWwiseIOHookImpl::Init(const AkDeviceSettings& InDeviceSettings)
{
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (LIKELY(ExternalSourceManager))
	{
		ExternalSourceManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	auto* MediaManager = IWwiseMediaManager::Get();
	if (LIKELY(MediaManager))
	{
		MediaManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (LIKELY(SoundBankManager))
	{
		SoundBankManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	return FWwiseDefaultIOHook::Init(InDeviceSettings);
}

AKRESULT FWwiseIOHookImpl::Open(
	const AkOSChar*			in_pszFileName,
	AkOpenMode				in_eOpenMode,
	AkFileSystemFlags*		in_pFlags,
	bool&					io_bSyncOpen,
	AkFileDesc&				out_fileDesc
)
{
	io_bSyncOpen = true;	// This can be run on any thread.

	const FString Filename(in_pszFileName);

	if (in_eOpenMode == AK_OpenModeWrite || in_eOpenMode == AK_OpenModeWriteOvrwr)
	{
		return OpenFileForWrite(in_pszFileName, in_eOpenMode, in_pFlags, io_bSyncOpen, out_fileDesc);
	}

	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
	UE_LOG(LogWwiseFileHandler, Error, TEXT("IO Hook doesn't support opening by Name: %s"), *Filename);
	return AK_NotImplemented;
}

AKRESULT FWwiseIOHookImpl::Open(
	AkFileID				in_fileID,          // File ID.
	AkOpenMode				in_eOpenMode,       // Open mode.
	AkFileSystemFlags*		in_pFlags,			// Special flags. Can pass NULL.
	bool&					io_bSyncOpen,		// If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
	AkFileDesc&				out_fileDesc        // Returned file descriptor.
)
{
	io_bSyncOpen = true;	// This can be run on any thread.

	if (in_eOpenMode != AK_OpenModeRead || !in_pFlags)
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Unsupported Open Mode for File ID %" PRIu32), in_fileID);
		return AK_NotImplemented;
	}

	IWwiseStreamingManagerHooks* StreamingHooks = GetStreamingHooks(*in_pFlags);
	if (UNLIKELY(!StreamingHooks))
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Unsupported Streaming for File ID %" PRIu32), in_fileID);
		return AK_NotInitialized;
	}

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Opening file for streaming: File ID %" PRIu32), in_fileID);
	const auto AkResult = StreamingHooks->OpenStreaming(out_fileDesc, in_fileID);
	if (UNLIKELY(AkResult != AK_Success))
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Streaming Open failed for File ID %" PRIu32 ": %" PRIi32 " (%s)"), in_fileID, AkResult, AkUnrealHelper::GetResultString(AkResult));
		return AK_NotImplemented;
	}

	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
	return AK_Success;
}

AKRESULT FWwiseIOHookImpl::Read(
	AkFileDesc&				in_fileDesc,
	const AkIoHeuristics&	in_heuristics,
	AkAsyncIOTransferInfo&	io_transferInfo
)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalRequests);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
	++CurrentDeviceData;
	if (CurrentDeviceData > MaxDeviceData)
	{
		MaxDeviceData.Store(CurrentDeviceData);
	}
#endif

	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(in_fileDesc);
	if (!FileState)
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("IOHook::Read: Could not find File Descriptor"));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
#ifndef AK_OPTIMIZED
		--CurrentDeviceData;
#endif
		return AK_IDNotFound;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IOHook::Read: Reading %" PRIu32 " bytes @ %" PRIu64 " - Priority %" PRIi8 " Deadline %f"), io_transferInfo.uRequestedSize, io_transferInfo.uFilePosition, in_heuristics.priority, (double)in_heuristics.fDeadline);
	const auto Result = FileState->ProcessRead(in_fileDesc, in_heuristics, io_transferInfo, 
		[this, OpCycleCounter = MoveTemp(OpCycleCounter)]
			(AkAsyncIOTransferInfo* InTransferInfo, AKRESULT InResult) mutable
		{
			ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
			if (UNLIKELY(InResult != AK_Success))
			{
				ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			}

#ifndef AK_OPTIMIZED
			--CurrentDeviceData;
#endif
			OpCycleCounter.Stop();

			if (UNLIKELY(!InTransferInfo->pCallback))
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("IOHook::Read: No callback reading data"));
			}
			FWwiseAsyncCycleCounter CallbackCycleCounter(GET_STATID(STAT_WwiseFileHandlerSoundEngineCallbackLatency));
			InTransferInfo->pCallback(InTransferInfo, InResult);
		});
	return Result;
}

AKRESULT FWwiseIOHookImpl::Write(
	AkFileDesc& in_fileDesc,
	const AkIoHeuristics& in_heuristics,
	AkAsyncIOTransferInfo& io_transferInfo
)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalRequests);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
	++CurrentDeviceData;
	if (CurrentDeviceData > MaxDeviceData)
	{
		MaxDeviceData.Store(CurrentDeviceData);
	}
#endif

	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(in_fileDesc);
	if (!FileState)
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("IOHook::Write: Could not find File Descriptor"));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
		--CurrentDeviceData;
#endif
		return AK_IDNotFound;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("IOHook::Write: Writing %" PRIu32 " bytes @ %" PRIu64), io_transferInfo.uBufferSize, io_transferInfo.uFilePosition);
	const auto Result = FileState->ProcessWrite(in_fileDesc, in_heuristics, io_transferInfo, 
		[this, OpCycleCounter = MoveTemp(OpCycleCounter)]
			(AkAsyncIOTransferInfo* InTransferInfo, AKRESULT InResult) mutable
		{
			ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
			if (UNLIKELY(InResult != AK_Success))
			{
				ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			}

#ifndef AK_OPTIMIZED
			--CurrentDeviceData;
#endif
			OpCycleCounter.Stop();

			if (UNLIKELY(!InTransferInfo->pCallback))
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("IOHook::Write: No callback reading data"));
			}
			FWwiseAsyncCycleCounter CallbackCycleCounter(GET_STATID(STAT_WwiseFileHandlerSoundEngineCallbackLatency));
			InTransferInfo->pCallback(InTransferInfo, InResult);
		});
	return Result;
}

void FWwiseIOHookImpl::Cancel(
	AkFileDesc& in_fileDesc,
	AkAsyncIOTransferInfo& io_transferInfo,
	bool& io_bCancelAllTransfersForThisFile
)
{
	UE_LOG(LogWwiseFileHandler, Log, TEXT("Cancelling transfer unsupported"));
//	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
}

AKRESULT FWwiseIOHookImpl::Close(AkFileDesc& in_fileDesc)
{
	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(in_fileDesc);
	if (!FileState)
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("Close: Could not find File Descriptor"));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		return AK_IDNotFound;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Closing streaming file"));
	FileState->CloseStreaming();
	ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
	return AK_Success;
}

// Returns the block size for the file or its storage device. 
AkUInt32 FWwiseIOHookImpl::GetBlockSize(AkFileDesc& in_fileDesc)
{
	return 1;
}

// Returns a description for the streaming device above this low-level hook.
void FWwiseIOHookImpl::GetDeviceDesc(AkDeviceDesc& out_deviceDesc)
{
#if !defined(AK_OPTIMIZED)
	// Deferred scheduler.
	out_deviceDesc.deviceID = m_deviceID;
	out_deviceDesc.bCanRead = true;
	out_deviceDesc.bCanWrite = true;
	AK_CHAR_TO_UTF16(out_deviceDesc.szDeviceName, "UnrealIODevice", AK_MONITOR_DEVICENAME_MAXLENGTH);
	out_deviceDesc.szDeviceName[AK_MONITOR_DEVICENAME_MAXLENGTH - 1] = '\0';
	out_deviceDesc.uStringSize = (AkUInt32)AKPLATFORM::AkUtf16StrLen(out_deviceDesc.szDeviceName) + 1;
#endif
}

// Returns custom profiling data: Current number of pending streaming requests
AkUInt32 FWwiseIOHookImpl::GetDeviceData()
{
#ifndef AK_OPTIMIZED
	AkUInt32 Result = MaxDeviceData;
	MaxDeviceData.Store(CurrentDeviceData);
#else
	AkUInt32 Result = 0;
#endif
	return Result;
}

IWwiseStreamingManagerHooks* FWwiseIOHookImpl::GetStreamingHooks(const AkFileSystemFlags& InFileSystemFlag)
{
	IWwiseStreamableFileHandler* WwiseStreamableFileHandler = nullptr;
	if (InFileSystemFlag.uCompanyID == AKCOMPANYID_AUDIOKINETIC_EXTERNAL)
	{
		WwiseStreamableFileHandler = IWwiseExternalSourceManager::Get();
	}
	else if (InFileSystemFlag.uCodecID == AKCODECID_BANK || InFileSystemFlag.uCodecID == AKCODECID_BANK_EVENT || InFileSystemFlag.uCodecID == AKCODECID_BANK_BUS)
	{
		WwiseStreamableFileHandler = IWwiseSoundBankManager::Get();
	}
	else
	{
		WwiseStreamableFileHandler = IWwiseMediaManager::Get();
	}

	if (UNLIKELY(!WwiseStreamableFileHandler))
	{
		return nullptr;
	}
	return &WwiseStreamableFileHandler->GetStreamingHooks();
}

AKRESULT FWwiseIOHookImpl::OpenFileForWrite(const AkOSChar* in_pszFileName, AkOpenMode in_eOpenMode,
	AkFileSystemFlags* in_pFlags, bool& io_bSyncOpen, AkFileDesc& out_fileDesc)
{
	const auto TargetDirectory = FPaths::ProjectSavedDir() / TEXT("Wwise");
	static bool TargetDirectoryExists = false;
	if (!TargetDirectoryExists && !FPaths::DirectoryExists(TargetDirectory))
	{
		TargetDirectoryExists = true;
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*TargetDirectory))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot create writable directory at %s"), *TargetDirectory);
			ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			return AK_NotImplemented;
		}
	}
	const auto FullPath = FPaths::Combine(TargetDirectory, FString(in_pszFileName));

	UE_LOG(LogWwiseFileHandler, Log, TEXT("Opening file for writing: %s"), *FullPath);
	const auto FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FullPath);
	if (UNLIKELY(!FileHandle))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot open file %s for write."), *FullPath);
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		return AK_UnknownFileError;
	}

	auto* WriteState = new FWwiseWriteFileState(FileHandle, FullPath);
	WriteState->GetFileDescCopy(out_fileDesc);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
	return AK_Success;
}
