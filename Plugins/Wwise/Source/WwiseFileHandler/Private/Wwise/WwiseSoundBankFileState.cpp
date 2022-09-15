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

#include "Wwise/WwiseSoundBankFileState.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"
#include "AkUnrealHelper.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseSoundBankFileState::FWwiseSoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath):
	FWwiseSoundBankCookedData(InCookedData),
	RootPath(InRootPath)
{
}

FWwiseInMemorySoundBankFileState::FWwiseInMemorySoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath) :
	FWwiseSoundBankFileState(InCookedData, InRootPath),
	Ptr(nullptr),
	FileSize(0),
	MappedHandle(nullptr),
	MappedRegion(nullptr)
{
}

bool FWwiseInMemorySoundBankFileState::LoadAsMemoryView() const
{
#if WITH_EDITOR
	return false;
#else
	return bContainsMedia;
#endif
}

void FWwiseInMemorySoundBankFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	if (UNLIKELY(FileSize != 0 || Ptr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Seems to be already opened."), SoundBankId, *DebugName);
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / SoundBankPathName;

	const bool bCanTryMemoryMapping = !bContainsMedia || (!bDeviceMemory && !MemoryAlignment);
	if (bCanTryMemoryMapping
		&& GetMemoryMapped(MappedHandle, MappedRegion, FileSize, FullPathName, 0))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Loading Memory Mapped SoundBank as %s."), SoundBankId, *DebugName, LoadAsMemoryView() ? TEXT("View") : TEXT("Copy"));
		INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedSoundBanks);
		Ptr = MappedRegion->GetMappedPtr();
		FileSize = MappedRegion->GetMappedSize();
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else if (LIKELY(GetFileToPtr(Ptr, FileSize, FullPathName, bDeviceMemory, MemoryAlignment, bContainsMedia)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Loading SoundBank as %s."), SoundBankId, *DebugName, LoadAsMemoryView() ? TEXT("View") : TEXT("Copy"));
		INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedSoundBanks);
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank (%s)."), SoundBankId, *DebugName, *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemorySoundBankFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(!FileSize || !Ptr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): No data, but supposed to be loaded."), SoundBankId, *DebugName);
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed loading SoundBank %" PRIu32 " (%s) without a SoundEngine."), SoundBankId, *DebugName);
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	AkBankID LoadedSoundBankId;
	AkBankType LoadedSoundBankType;
	const auto LoadResult = 
		LoadAsMemoryView()
			? SoundEngine->LoadBankMemoryView(Ptr, FileSize, LoadedSoundBankId, LoadedSoundBankType)
			: SoundEngine->LoadBankMemoryCopy(Ptr, FileSize, LoadedSoundBankId, LoadedSoundBankType);

	if (LIKELY(LoadResult == AK_Success))
	{
		UE_CLOG(UNLIKELY(LoadedSoundBankId != SoundBankId), LogWwiseFileHandler, Error, TEXT("Incorrect SoundBank loaded: %" PRIu32 " expected %" PRIu32 " (%s)"), LoadedSoundBankId, SoundBankId, *DebugName);
		UE_CLOG(UNLIKELY(LoadedSoundBankType != static_cast<uint8>(SoundBankType)), LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Incorrect SoundBank type: %" PRIu8 " expected %" PRIu8), SoundBankId, *DebugName, (uint8)LoadedSoundBankType, (uint8)SoundBankType);

		// We don't need the memory anymore if we copied it
		if (!LoadAsMemoryView())
		{
			if (MappedHandle)
			{
				UnmapRegion(*MappedRegion);
				UnmapHandle(*MappedHandle);

				MappedRegion = nullptr;
				MappedHandle = nullptr;
			}
			else
			{
				DeallocateMemory(Ptr, FileSize, bDeviceMemory, MemoryAlignment, bContainsMedia);
			}
			Ptr = nullptr;
			FileSize = 0;
		}

		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedSoundBanks);
		LoadInSoundEngineSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: %d (%s)."), SoundBankId, *DebugName, LoadResult, AkUnrealHelper::GetResultString(LoadResult));
		LoadInSoundEngineFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemorySoundBankFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s) without a SoundEngine."), SoundBankId, *DebugName);
		return CloseFileDone(MoveTemp(InCallback));
	}

	const auto Result = SoundEngine->UnloadBank(SoundBankId, Ptr, static_cast<AkBankType>(SoundBankType));
	if (UNLIKELY(Result == AK_ResourceInUse))
	{
		UnloadFromSoundEngineDefer(MoveTemp(InCallback));
	}
	else
	{
		UE_CLOG(Result != AK_Success, LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): UnloadBank failed: %d (%s)"), SoundBankId, *DebugName, Result, AkUnrealHelper::GetResultString(Result));
		DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedSoundBanks);
		if (Ptr)
		{
			UnloadFromSoundEngineDone(MoveTemp(InCallback));
		}
		else
		{
			DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedSoundBanks);
			UnloadFromSoundEngineToClosedFile(MoveTemp(InCallback));
		}
	}
}

bool FWwiseInMemorySoundBankFileState::CanCloseFile() const
{
	// LoadFromSoundEngine will copy and delete the pointer. If succeeded, we are already closed.
	return (State == EState::Opened && Ptr == nullptr) || FWwiseSoundBankFileState::CanCloseFile();
}

void FWwiseInMemorySoundBankFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Closing Memory Mapped SoundBank."), SoundBankId, *DebugName);
	if (MappedHandle)
	{
		UnmapRegion(*MappedRegion);
		UnmapHandle(*MappedHandle);

		MappedRegion = nullptr;
		MappedHandle = nullptr;
	}
	else if (Ptr)
	{
		DeallocateMemory(Ptr, FileSize, bDeviceMemory, MemoryAlignment, bContainsMedia);
	}
	Ptr = nullptr;
	FileSize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

