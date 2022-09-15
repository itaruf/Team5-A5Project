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

#include "Wwise/WwiseMediaManagerImpl.h"
#include "Wwise/WwiseMediaFileState.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Async/Async.h"

UWwiseMediaManagerImpl::UWwiseMediaManagerImpl()
{
}

UWwiseMediaManagerImpl::~UWwiseMediaManagerImpl()
{
}

void UWwiseMediaManagerImpl::LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback)
{
	IncrementFileStateUseAsync(InMediaCookedData.MediaId, EWwiseFileStateOperationOrigin::Loading, [this, InMediaCookedData, InRootPath]() mutable
	{
		return CreateOp(InMediaCookedData, InRootPath);
	}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
	{
		InCallback(bInResult);
	});
}

void UWwiseMediaManagerImpl::UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback)
{
	DecrementFileStateUseAsync(InMediaCookedData.MediaId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
}

void UWwiseMediaManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	StreamingGranularity = InStreamingGranularity;
}

FWwiseFileStateSharedPtr UWwiseMediaManagerImpl::CreateOp(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath)
{
	if (InMediaCookedData.bStreaming)
	{
		return FWwiseFileStateSharedPtr(new FWwiseStreamingMediaFileState(InMediaCookedData, InRootPath, StreamingGranularity));
	}
	else
	{
		return FWwiseFileStateSharedPtr(new FWwiseInMemoryMediaFileState(InMediaCookedData, InRootPath));
	}
}
