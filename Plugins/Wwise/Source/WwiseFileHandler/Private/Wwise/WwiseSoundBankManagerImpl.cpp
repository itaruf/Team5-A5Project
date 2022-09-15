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

#include "Wwise/WwiseSoundBankManagerImpl.h"
#include "Wwise/WwiseSoundBankFileState.h"

UWwiseSoundBankManagerImpl::UWwiseSoundBankManagerImpl() :
	StreamingGranularity(0)
{
}

UWwiseSoundBankManagerImpl::~UWwiseSoundBankManagerImpl()
{
}

void UWwiseSoundBankManagerImpl::LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback)
{
	IncrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, EWwiseFileStateOperationOrigin::Loading, [this, InSoundBankCookedData, InRootPath]() mutable
	{
		return CreateOp(InSoundBankCookedData, InRootPath);
	}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
	{
		InCallback(bInResult);
	});
}

void UWwiseSoundBankManagerImpl::UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback)
{
	DecrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
}

void UWwiseSoundBankManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	StreamingGranularity = InStreamingGranularity;
}

FWwiseFileStateSharedPtr UWwiseSoundBankManagerImpl::CreateOp(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath)
{
	return FWwiseFileStateSharedPtr(new FWwiseInMemorySoundBankFileState(InSoundBankCookedData, InRootPath));
}
