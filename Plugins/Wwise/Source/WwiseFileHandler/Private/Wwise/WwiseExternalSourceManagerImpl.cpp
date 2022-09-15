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

#include "Wwise/WwiseExternalSourceManagerImpl.h"
#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include <inttypes.h>

FWwiseExternalSourceState::FWwiseExternalSourceState(const FWwiseExternalSourceCookedData& InCookedData) :
	FWwiseExternalSourceCookedData(InCookedData),
	LoadCount(0)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerCreatedExternalSourceStates);
}

FWwiseExternalSourceState::~FWwiseExternalSourceState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerCreatedExternalSourceStates);
}

void FWwiseExternalSourceState::IncrementLoadCount()
{
	++LoadCount;
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): ++LoadCount=%d"), Cookie, *DebugName, LoadCount);
}

bool FWwiseExternalSourceState::DecrementLoadCount()
{
	const bool bResult = --LoadCount == 0;
	if (bResult)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): --LoadCount=%d. Deleting."), Cookie, *DebugName, LoadCount);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): --LoadCount=%d"), Cookie, *DebugName, LoadCount);
	}
	return bResult;
}


UWwiseExternalSourceManagerImpl::UWwiseExternalSourceManagerImpl() :
	StreamingGranularity(0)
{
}

UWwiseExternalSourceManagerImpl::~UWwiseExternalSourceManagerImpl()
{
}

void UWwiseExternalSourceManagerImpl::LoadExternalSource(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
	const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async([this, InExternalSourceCookedData, InRootPath, InLanguage, InCallback = MoveTemp(InCallback)]() mutable
	{
		LoadExternalSourceImpl(InExternalSourceCookedData, InRootPath, InLanguage, MoveTemp(InCallback));
	});
}

void UWwiseExternalSourceManagerImpl::UnloadExternalSource(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath,
	const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async([this, InExternalSourceCookedData, InRootPath, InLanguage, InCallback = MoveTemp(InCallback)]() mutable
	{
		UnloadExternalSourceImpl(InExternalSourceCookedData, InRootPath, InLanguage, MoveTemp(InCallback));
	});
}

void UWwiseExternalSourceManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	StreamingGranularity = InStreamingGranularity;
}

bool UWwiseExternalSourceManagerImpl::GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const FWwiseExternalSourceCookedData& InCookedData)
{
	return GetExternalSourceInfo(OutInfo, InCookedData.Cookie, InCookedData.DebugName);
}

bool UWwiseExternalSourceManagerImpl::GetExternalSourceInfo(AkExternalSourceInfo& OutInfo, const uint32 InExternalSourceCookie, const FString& InExternalSourceName)
{
	bool bResult = true;
	FileHandlerExecutionQueue.AsyncWait([this, &bResult, &OutInfo, InExternalSourceCookie, InExternalSourceName]() mutable
	{
		bResult = GetExternalSourceInfoImpl(OutInfo, InExternalSourceCookie, InExternalSourceName);
	});
	return bResult;
}

bool UWwiseExternalSourceManagerImpl::GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo, const TArray<uint32>& InExternalSourceCookies, const TArray<FString>& InExternalSourceNames)
{
	bool bResult = true;

	if (InExternalSourceCookies.Num() > 0)
		FileHandlerExecutionQueue.AsyncWait([this, &bResult, &OutInfo, &InExternalSourceCookies, &InExternalSourceNames]() mutable
	{
		OutInfo.Reset(InExternalSourceCookies.Num());
		for (int i = 0; i < InExternalSourceCookies.Num(); i++)
		{
			const auto& ExternalSourceCookie = InExternalSourceCookies[i];
			const FString ExternalSourceName = InExternalSourceNames.Num() >=i ? InExternalSourceNames[i] : TEXT("Unknown");
			AkExternalSourceInfo Info;
			if (LIKELY(GetExternalSourceInfoImpl(Info, ExternalSourceCookie, ExternalSourceName)))
			{
				//Set the Cookie here (because the ext src media state can't definitively know the cookie)
				auto InfoCopy = MoveTemp(Info);
				InfoCopy.iExternalSrcCookie = ExternalSourceCookie;
				OutInfo.Add(InfoCopy);  
			}
			else
			{
				bResult = false;
			}
		}

		if (UNLIKELY(!bResult))
		{
			if (OutInfo.Num() > 0)
			{
				UE_LOG(LogWwiseFileHandler, Log, TEXT("GetExternalSourceInfos: Partially successful (%d expected %d)."), OutInfo.Num(), InExternalSourceCookies.Num());
				bResult = true;
			}
			else
			{
				UE_LOG(LogWwiseFileHandler, Log, TEXT("GetExternalSourceInfos: Unsuccessful."));
			}
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("GetExternalSourceInfos: Successfuly retrieved requested %d external sources."), OutInfo.Num());
		}
	});
	return bResult;
}

bool UWwiseExternalSourceManagerImpl::GetExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo,
	const TArray<FWwiseExternalSourceCookedData>& InCookedData)
{
	TArray<uint32> ExternalSourceCookies;
	TArray<FString> ExternalSourceNames;

	bool bSuccess = true;

	OutInfo.Reset(InCookedData.Num());
	for (const auto& CookedData : InCookedData)
	{
		ExternalSourceCookies.Add(CookedData.Cookie);
		ExternalSourceNames.Add(CookedData.DebugName);
	}

	return GetExternalSourceInfos(OutInfo, ExternalSourceCookies, ExternalSourceNames);
}

#if WITH_EDITORONLY_DATA
void UWwiseExternalSourceManagerImpl::Cook(UWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData,
	TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
	const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("UWwiseExternalSourceManagerImpl::Cook: External Source manager needs to be overridden."));
}
#endif

void UWwiseExternalSourceManagerImpl::LoadExternalSourceImpl(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath, const FWwiseLanguageCookedData& InLanguage,
	FLoadExternalSourceCallback&& InCallback)
{
	FWwiseExternalSourceStateSharedPtr State;
	if (const auto* StatePtr = ExternalSourceStatesById.Find(InExternalSourceCookedData.Cookie))
	{
		State = *StatePtr;
		State->IncrementLoadCount();
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Creating new State for %s %" PRIu32), GetManagingTypeName(), InExternalSourceCookedData.Cookie);
		State = CreateExternalSourceState(InExternalSourceCookedData, InRootPath);
		if (UNLIKELY(!State.IsValid()))
		{
			InCallback(false);
			return;
		}
		else
		{
			State->IncrementLoadCount();
			ExternalSourceStatesById.Add(InExternalSourceCookedData.Cookie, State);
		}
	}
	LoadExternalSourceMedia(InExternalSourceCookedData.Cookie, InExternalSourceCookedData.DebugName, InRootPath, MoveTemp(InCallback));
}

void UWwiseExternalSourceManagerImpl::UnloadExternalSourceImpl(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath, const FWwiseLanguageCookedData& InLanguage,
	FUnloadExternalSourceCallback&& InCallback)
{
	FWwiseExternalSourceStateSharedPtr State;
	if (const auto* StatePtr = ExternalSourceStatesById.Find(InExternalSourceCookedData.Cookie))
	{
		State = *StatePtr;
	}

	if (UNLIKELY(!State.IsValid()))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("ExternalSource %" PRIu32 " (%s): Unloading an unknown External Source"), InExternalSourceCookedData.Cookie, *InExternalSourceCookedData.DebugName);
		InCallback();
	}
	else
	{
		FWwiseExternalSourceState* ExternalSourceState = State.Get();
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource %" PRIu32 " (%s): Closing State instance"), InExternalSourceCookedData.Cookie, *InExternalSourceCookedData.DebugName);
		if (CloseExternalSourceState(*State) && InExternalSourceCookedData.Cookie != 0)
		{
			ExternalSourceStatesById.Remove(InExternalSourceCookedData.Cookie);
			State.Reset();
		}
		if (LIKELY(InExternalSourceCookedData.Cookie != 0))
		{
			UnloadExternalSourceMedia(InExternalSourceCookedData.Cookie, InExternalSourceCookedData.DebugName, InRootPath, MoveTemp(InCallback));
		}
		else
		{
			InCallback();
		}
	}
}

FWwiseExternalSourceStateSharedPtr UWwiseExternalSourceManagerImpl::CreateExternalSourceState(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FString& InRootPath)
{
	return FWwiseExternalSourceStateSharedPtr(new FWwiseExternalSourceState(InExternalSourceCookedData));
}

bool UWwiseExternalSourceManagerImpl::CloseExternalSourceState(FWwiseExternalSourceState& InExternalSourceState)
{
	return InExternalSourceState.DecrementLoadCount();
}


void UWwiseExternalSourceManagerImpl::LoadExternalSourceMedia(const uint32 InExternalSourceCookie,
	const FString& InExternalSourceName, const FString& InRootPath, FLoadExternalSourceCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
	InCallback(false);
}

void UWwiseExternalSourceManagerImpl::UnloadExternalSourceMedia(const uint32 InExternalSourceCookie,
	const FString& InExternalSourceName, const FString& InRootPath, FUnloadExternalSourceCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
	InCallback();
}

bool UWwiseExternalSourceManagerImpl::GetExternalSourceInfoImpl(AkExternalSourceInfo& OutInfo,
	uint32 InExternalSourceCookie, const FString& ExternalSourceName)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
	return false;
}

void UWwiseExternalSourceManagerImpl::OnPostEvent(const uint32 InPlayingID,
                                                  const TArray<AkExternalSourceInfo>& InExternalSources)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
}

void UWwiseExternalSourceManagerImpl::OnEndOfEvent(const uint32 InPlayingID)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
}


