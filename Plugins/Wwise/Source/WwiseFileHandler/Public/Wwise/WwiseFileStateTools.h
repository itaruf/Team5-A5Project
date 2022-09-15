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

#include "Wwise/WwiseExecutionQueue.h"
#include "AkInclude.h"

class WWISEFILEHANDLER_API FWwiseFileStateTools
{
public:
	using TFileOpDoneCallback = TUniqueFunction<void(AkAsyncIOTransferInfo* InTransferInfo, AKRESULT InResult)>;

	virtual ~FWwiseFileStateTools() {}

protected:
	FWwiseExecutionQueue FileStateExecutionQueue;
	
	static uint8* AllocateMemory(int64 InMemorySize, bool bInDeviceMemory, int32 InMemoryAlignment, bool bInEnforceMemoryRequirements);
	static void DeallocateMemory(const uint8* InMemoryPtr, int64 InMemorySize, bool bInDeviceMemory, int32 InMemoryAlignment, bool bInEnforceMemoryRequirements);

	static bool GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, IMappedFileRegion*& OutMappedRegion, int64& OutSize,
		const FString& InFilePathname, int32 InMemoryAlignment);
	static bool GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, int64& OutSize,
		const FString& InFilePathname, int32 InMemoryAlignment);
	static bool GetMemoryMappedRegion(IMappedFileRegion*& OutMappedRegion, IMappedFileHandle& InMappedHandle);
	static void UnmapRegion(IMappedFileRegion& InMappedRegion);
	static void UnmapHandle(IMappedFileHandle& InMappedHandle);

	static bool GetFileToPtr(const uint8*& OutPtr, int64& OutSize,
		const FString& InFilePathname, bool bInDeviceMemory, int32 InMemoryAlignment, bool bInEnforceMemoryRequirements);
	static bool GetFileArchive(FArchive*& OutArchive, const FString& InFilePathname);

	static bool Read(FArchive& InArchive, uint8* OutBuffer, int64 InPosition, uint32 InSize);

	void AsyncRead(FArchive& InArchive, FWwiseExecutionQueue::FTimedCallback&& InCallback,
		uint8* OutBuffer, int64 InPosition, uint32 InSize,
		FTimespan InDeadline = FWwiseExecutionQueue::NoTimeLimit(), AkPriority InPriority = AK_DEFAULT_PRIORITY);

	void AsyncRead(FArchive& InArchive, const TCHAR* InAssetType, uint32 InAssetId, const TCHAR* InAssetName,
		const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, TFileOpDoneCallback&& InFileOpDoneCallback);

	void AsyncClose(FArchive* InArchive, FWwiseExecutionQueue::FBasicFunction&& InCallback);
};
