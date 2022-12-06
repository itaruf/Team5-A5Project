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

#include "Wwise/WwiseSoundBankFileState.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"
#include "AkUnrealHelper.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseSoundBankFileState::FWwiseSoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath):
	FWwiseSoundBankCookedData(InCookedData),
	RootPath(InRootPath)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerKnownSoundBanks);
}

FWwiseSoundBankFileState::~FWwiseSoundBankFileState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerKnownSoundBanks);
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
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Seems to be already opened."), SoundBankId, *DebugName.ToString());
		OpenFileFailed(MoveTemp(InCallback));
		return;
	}

	const auto FullPathName = RootPath / SoundBankPathName.ToString();

	const bool bCanTryMemoryMapping = !bContainsMedia || (!bDeviceMemory && !MemoryAlignment);
	if (bCanTryMemoryMapping
		&& GetMemoryMapped(MappedHandle, MappedRegion, FileSize, FullPathName, 0))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Loading Memory Mapped SoundBank as %s."), SoundBankId, *DebugName.ToString(), LoadAsMemoryView() ? TEXT("View") : TEXT("Copy"));
		Ptr = MappedRegion->GetMappedPtr();
		FileSize = MappedRegion->GetMappedSize();
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else if (LIKELY(GetFileToPtr(Ptr, FileSize, FullPathName, bDeviceMemory, MemoryAlignment, bContainsMedia)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Loading SoundBank as %s."), SoundBankId, *DebugName.ToString(), LoadAsMemoryView() ? TEXT("View") : TEXT("Copy"));
		OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank (%s)."), SoundBankId, *DebugName.ToString(), *FullPathName);
		OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemorySoundBankFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	if (UNLIKELY(!FileSize || !Ptr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): No data, but supposed to be loaded."), SoundBankId, *DebugName.ToString());
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed loading SoundBank %" PRIu32 " (%s) without a SoundEngine."), SoundBankId, *DebugName.ToString());
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	AkBankID LoadedSoundBankId;
	AkBankType LoadedSoundBankType;

	BankLoadCookie* Cookie = new BankLoadCookie(this);
	if (!Cookie)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: Could not allocate cookie."), SoundBankId, *DebugName.ToString());
		LoadInSoundEngineFailed(MoveTemp(InCallback));
		return;
	}

	Cookie->Callback = MoveTemp(InCallback);
	const auto LoadResult =
		LoadAsMemoryView()
			? SoundEngine->LoadBankMemoryView(Ptr, FileSize, &FWwiseInMemorySoundBankFileState::BankLoadCallback, Cookie, LoadedSoundBankId, LoadedSoundBankType)
			: SoundEngine->LoadBankMemoryCopy(Ptr, FileSize, &FWwiseInMemorySoundBankFileState::BankLoadCallback, Cookie, LoadedSoundBankId, LoadedSoundBankType);

	UE_CLOG(UNLIKELY(LoadedSoundBankType != static_cast<uint8>(SoundBankType)), LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Incorrect SoundBank type: %" PRIu8 " expected %" PRIu8), SoundBankId, *DebugName.ToString(), (uint8)LoadedSoundBankType, (uint8)SoundBankType);
	if(LoadResult != AK_Success)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: %d (%s)."), SoundBankId, *DebugName.ToString(), LoadResult, AkUnrealHelper::GetResultString(LoadResult));
		auto Callback = MoveTemp(Cookie->Callback);
		delete Cookie;
		LoadInSoundEngineFailed(MoveTemp(Callback));
		return;
	}
}

void FWwiseInMemorySoundBankFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s) without a SoundEngine."), SoundBankId, *DebugName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	BankUnloadCookie* Cookie = new BankUnloadCookie(this);
	if(!Cookie)
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s) could not allocate cookie for unload operation."), SoundBankId, *DebugName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	Cookie->Callback = MoveTemp(InCallback);
	const auto Result = SoundEngine->UnloadBank(SoundBankId, Ptr, &FWwiseInMemorySoundBankFileState::BankUnloadCallback, Cookie, static_cast<AkBankType>(SoundBankType));
	if(Result != AK_Success)
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Failed UnloadFromSoundEngine %" PRIu32 " (%s). Call to SoundEngine failed with result %s"), SoundBankId, *DebugName.ToString(), AkUnrealHelper::GetResultString(Result));
		auto Callback = MoveTemp(Cookie->Callback);
		delete Cookie;
		CloseFileDone(MoveTemp(Callback));
		return;
	}
}

bool FWwiseInMemorySoundBankFileState::CanCloseFile() const
{
	// LoadFromSoundEngine will copy and delete the pointer. If succeeded, we are already closed.
	return (State == EState::Opened && Ptr == nullptr) || FWwiseSoundBankFileState::CanCloseFile();
}

void FWwiseInMemorySoundBankFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("SoundBank %" PRIu32 " (%s): Closing Memory Mapped SoundBank."), SoundBankId, *DebugName.ToString());
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

void FWwiseInMemorySoundBankFileState::FreeMemoryIfNeeded()
{
	// We don't need the memory anymore if we copied it, whether the load succeeded or not.
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
}

void FWwiseInMemorySoundBankFileState::BankLoadCallback(
	AkUInt32	InBankID,
	const void*	InMemoryBankPtr,
	AKRESULT	InLoadResult,
	void*		InCookie
	)
{
	if (!InCookie)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: %d. Cookie given by SoundEngine is invalid."), InBankID, InLoadResult, AkUnrealHelper::GetResultString(InLoadResult));
		return;
	}

	AsyncTask(ENamedThreads::AnyThread, [=]() {
		BankLoadCookie Cookie((BankLoadCookie*)InCookie);
		delete (BankLoadCookie*)InCookie;
		if (!Cookie.BankFileState)
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: %d. Cookie given by SoundEngine is invalid."), InBankID, InLoadResult, AkUnrealHelper::GetResultString(InLoadResult));
			return;
		}

		auto* BankFileState = Cookie.BankFileState;
		if (LIKELY(InLoadResult == AK_Success))
		{
			UE_CLOG(UNLIKELY(InBankID != BankFileState->SoundBankId), LogWwiseFileHandler, Error, TEXT("Incorrect SoundBank loaded: %" PRIu32 " expected %" PRIu32 " (%s)"), InBankID, BankFileState->SoundBankId, *BankFileState->DebugName.ToString());

			INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedSoundBanks);
			BankFileState->FreeMemoryIfNeeded();
			BankFileState->LoadInSoundEngineSucceeded(MoveTemp(Cookie.Callback));
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): Failed to load SoundBank: %d (%s)."), InBankID, *BankFileState->DebugName.ToString(), InLoadResult, AkUnrealHelper::GetResultString(InLoadResult));
			BankFileState->FreeMemoryIfNeeded();
			BankFileState->LoadInSoundEngineFailed(MoveTemp(Cookie.Callback));
		}
	});
}

void FWwiseInMemorySoundBankFileState::BankUnloadCallback(
	AkUInt32	InBankID,
	const void* InMemoryBankPtr,
	AKRESULT	InUnloadResult,
	void*		InCookie
)
{
	if (!InCookie)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Failed UnloadFromSoundEngine %" PRIu32 ". Cookie given by SoundEngine is invalid."), InBankID);
		return;
	}

	AsyncTask(ENamedThreads::AnyThread, [=]() {
		BankUnloadCookie Cookie((BankUnloadCookie*)InCookie);
		delete (BankUnloadCookie*)InCookie;

		if (!Cookie.BankFileState)
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Failed UnloadFromSoundEngine %" PRIu32 ". Cookie given by SoundEngine is invalid."), InBankID);
			return;
		}

		auto* BankFileState = Cookie.BankFileState;
		if (UNLIKELY(InUnloadResult == AK_ResourceInUse))
		{
			BankFileState->UnloadFromSoundEngineDefer(MoveTemp(Cookie.Callback));
		}
		else
		{
			UE_CLOG(InUnloadResult != AK_Success, LogWwiseFileHandler, Error, TEXT("SoundBank %" PRIu32 " (%s): UnloadBank failed: %d (%s)"), InBankID, *BankFileState->DebugName.ToString(), InUnloadResult, AkUnrealHelper::GetResultString(InUnloadResult));
			DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedSoundBanks);
			if (InMemoryBankPtr)
			{
				BankFileState->UnloadFromSoundEngineDone(MoveTemp(Cookie.Callback));
			}
			else
			{
				BankFileState->UnloadFromSoundEngineToClosedFile(MoveTemp(Cookie.Callback));
			}
		}
	});
}

