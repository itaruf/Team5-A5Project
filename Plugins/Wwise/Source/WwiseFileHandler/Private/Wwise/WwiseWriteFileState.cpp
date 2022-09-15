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

#include "Wwise/WwiseWriteFileState.h"

#include <inttypes.h>

FWwiseWriteFileState::FWwiseWriteFileState(IFileHandle* InFileHandle, const FString& InFilePathName):
	FileHandle(InFileHandle),
	FilePathName(InFilePathName)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Creating writable file %s"), *FilePathName);
}

void FWwiseWriteFileState::CloseStreaming()
{
	FileStateExecutionQueue.AsyncWait([this]
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("ProcessWrite: Closing file %s"), *FilePathName);
		if (FileHandle)
		{
			delete FileHandle;
			FileHandle = nullptr;
		}
	});
	delete this;
}

AKRESULT FWwiseWriteFileState::ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback)
{
	FileStateExecutionQueue.Async([this, InFileDesc, InHeuristics, OutTransferInfo, InFileOpDoneCallback = MoveTemp(InFileOpDoneCallback)]() mutable
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ProcessWrite: Writing %" PRIu32 " bytes @ %" PRIu64 " in file %s"),
			OutTransferInfo.uBufferSize, OutTransferInfo.uFilePosition, *FilePathName);

		FileHandle->Seek(OutTransferInfo.uFilePosition);

		AKRESULT Result;
		if (LIKELY(FileHandle->Write(static_cast<uint8*>(OutTransferInfo.pBuffer), OutTransferInfo.uBufferSize)))
		{
			Result = AK_Success;
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("ProcessWrite: Failed writing %" PRIu32 " bytes @ %" PRIu64 " in file %s"),
				OutTransferInfo.uBufferSize, OutTransferInfo.uFilePosition, *FilePathName);

			Result = AK_UnknownFileError;
		}
		InFileOpDoneCallback(&OutTransferInfo, Result);
		return Result;
	});

	return AK_Success;
}
