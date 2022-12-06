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

#include "Wwise/WwiseResourceLoader.h"

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"

#include "Async/Async.h"

#if WITH_EDITORONLY_DATA && PLATFORM_WINDOWS
static const auto DefaultPlatform = MakeShared<FWwisePlatformId>(FGuid(0x6E0CB257, 0xC6C84C5C, 0x83662740, 0xDFC441EC), TEXT("Windows"), TEXT("Windows"));
#elif WITH_EDITORONLY_DATA && PLATFORM_MAC
static const auto DefaultPlatform = MakeShared<FWwisePlatformId>(FGuid(0x02DC7702, 0x6E7B4AE4, 0xBAE464D2, 0xB1057150), TEXT("Mac"), TEXT("Mac"));
#elif WITH_EDITORONLY_DATA && PLATFORM_LINUX
static const auto DefaultPlatform = MakeShared<FWwisePlatformId>(FGuid(0xBD0BDF13, 0x3125454F, 0x8BFD3195, 0x37169F81), TEXT("Linux"), TEXT("Linux"));
#else
static const auto DefaultPlatform = MakeShared<FWwisePlatformId>(FGuid(0x6E0CB257, 0xC6C84C5C, 0x83662740, 0xDFC441EC), TEXT("Windows"));
#endif

static const FWwiseLanguageCookedData DefaultLanguage(684519430, TEXT("English(US)"), EWwiseLanguageRequirement::IsDefault);


bool FWwiseResourceLoader::IsEnabled() const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return false;
	}

	return	ResourceLoaderImpl->IsEnabled();
}

void FWwiseResourceLoader::Enable()
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Enabling ResourceLoaderImpl..."));
	ResourceLoaderImpl->Enable();
}

void FWwiseResourceLoader::Disable()
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Disabling ResourceLoaderImpl..."));
	ResourceLoaderImpl->Disable();
}

FWwiseResourceLoader::FWwiseResourceLoader() :
	ResourceLoaderImpl(FWwiseResourceLoaderImpl::Instantiate())
{
	ResourceLoaderImpl->CurrentLanguage = SystemLanguage();
	ResourceLoaderImpl->CurrentPlatform = SystemPlatform();
}

void FWwiseResourceLoader::SetLanguage(FWwiseLanguageCookedData InLanguage, EWwiseReloadLanguage InReloadLanguage)
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return;
	}

	ResourceLoaderImpl->SetLanguage(InLanguage, InReloadLanguage);
}

void FWwiseResourceLoader::SetPlatform(const FWwiseSharedPlatformId& InPlatform)
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return;
	}

	ResourceLoaderImpl->SetPlatform(InPlatform);
}


FWwiseLanguageCookedData FWwiseResourceLoader::GetCurrentLanguage() const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->CurrentLanguage;
}

FWwiseSharedPlatformId FWwiseResourceLoader::GetCurrentPlatform() const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->CurrentPlatform;
}

FString FWwiseResourceLoader::GetUnrealPath(const FString& InPath) const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->GetUnrealPath(InPath);
}

FName FWwiseResourceLoader::GetUnrealExternalSourcePath() const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->GetUnrealExternalSourcePath();
}

FString FWwiseResourceLoader::GetUnrealStagePath(const FString& InPath) const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->GetUnrealStagePath(InPath);
}

#if WITH_EDITORONLY_DATA
FString FWwiseResourceLoader::GetUnrealGeneratedSoundBanksPath(const FString& InPath) const
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return {};
	}

	return ResourceLoaderImpl->GetUnrealGeneratedSoundBanksPath(InPath);
}

void FWwiseResourceLoader::SetUnrealGeneratedSoundBanksPath(const FDirectoryPath& DirectoryPath)
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		return;
	}

	ResourceLoaderImpl->GeneratedSoundBanksPath = DirectoryPath;
}

const FDirectoryPath& FWwiseResourceLoader::GetUnrealGeneratedSoundBanksPath()
{
	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		static const FDirectoryPath Empty;
		return Empty;
	}

	return ResourceLoaderImpl->GeneratedSoundBanksPath;
}
#endif

//
// User-facing loading and unloading operations
//

FWwiseLoadedAuxBus FWwiseResourceLoader::LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData,
	const FWwiseLanguageCookedData* InLanguageOverride)
{
	return LoadAuxBusAsync(InAuxBusCookedData, InLanguageOverride).Get();
}

void FWwiseResourceLoader::UnloadAuxBus(FWwiseLoadedAuxBus&& InAuxBus)
{
	FWwiseLoadedAuxBusPromise Promise;
	Promise.EmplaceValue(MoveTemp(InAuxBus));
	UnloadAuxBusAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedEvent FWwiseResourceLoader::LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData,
	const FWwiseLanguageCookedData* InLanguageOverride)
{
	return LoadEventAsync(InEventCookedData, InLanguageOverride).Get();
}

void FWwiseResourceLoader::UnloadEvent(FWwiseLoadedEvent&& InEvent)
{
	FWwiseLoadedEventPromise Promise;
	Promise.EmplaceValue(MoveTemp(InEvent));
	UnloadEventAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedExternalSource FWwiseResourceLoader::LoadExternalSource(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	return LoadExternalSourceAsync(InExternalSourceCookedData).Get();
}

void FWwiseResourceLoader::UnloadExternalSource(FWwiseLoadedExternalSource&& InExternalSource)
{
	FWwiseLoadedExternalSourcePromise Promise;
	Promise.EmplaceValue(MoveTemp(InExternalSource));
	UnloadExternalSourceAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedGroupValue FWwiseResourceLoader::LoadGroupValue(const FWwiseGroupValueCookedData& InGroupValueCookedData)
{
	return LoadGroupValueAsync(InGroupValueCookedData).Get();
}

void FWwiseResourceLoader::UnloadGroupValue(FWwiseLoadedGroupValue&& InGroupValue)
{
	FWwiseLoadedGroupValuePromise Promise;
	Promise.EmplaceValue(MoveTemp(InGroupValue));
	UnloadGroupValueAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedInitBank FWwiseResourceLoader::LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData)
{
	return LoadInitBankAsync(InInitBankCookedData).Get();
}

void FWwiseResourceLoader::UnloadInitBank(FWwiseLoadedInitBank&& InInitBank)
{
	FWwiseLoadedInitBankPromise Promise;
	Promise.EmplaceValue(MoveTemp(InInitBank));
	UnloadInitBankAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedMedia FWwiseResourceLoader::LoadMedia(const FWwiseMediaCookedData& InMediaCookedData)
{
	return LoadMediaAsync(InMediaCookedData).Get();
}

void FWwiseResourceLoader::UnloadMedia(FWwiseLoadedMedia&& InMedia)
{
	FWwiseLoadedMediaPromise Promise;
	Promise.EmplaceValue(MoveTemp(InMedia));
	UnloadMediaAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedShareSet FWwiseResourceLoader::LoadShareSet(const FWwiseLocalizedShareSetCookedData& InShareSetCookedData,
	const FWwiseLanguageCookedData* InLanguageOverride)
{
	return LoadShareSetAsync(InShareSetCookedData, InLanguageOverride).Get();
}

void FWwiseResourceLoader::UnloadShareSet(FWwiseLoadedShareSet&& InShareSet)
{
	FWwiseLoadedShareSetPromise Promise;
	Promise.EmplaceValue(MoveTemp(InShareSet));
	UnloadShareSetAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedSoundBank FWwiseResourceLoader::LoadSoundBank(
	const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	return LoadSoundBankAsync(InSoundBankCookedData, InLanguageOverride).Get();
}

void FWwiseResourceLoader::UnloadSoundBank(FWwiseLoadedSoundBank&& InSoundBank)
{
	FWwiseLoadedSoundBankPromise Promise;
	Promise.EmplaceValue(MoveTemp(InSoundBank));
	UnloadSoundBankAsync(Promise.GetFuture().Share()).Wait();
}

FWwiseLoadedAuxBusFuture FWwiseResourceLoader::LoadAuxBusAsync(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData,
                                                         const FWwiseLanguageCookedData* InLanguageOverride)
{
	FWwiseLoadedAuxBusPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* AuxBusNode = ResourceLoaderImpl->CreateAuxBusNode(InAuxBusCookedData, InLanguageOverride);
		if (UNLIKELY(!AuxBusNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadAuxBusAsync(MoveTemp(Promise), MoveTemp(AuxBusNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadAuxBusAsync(FWwiseLoadedAuxBusFuture&& InAuxBus)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InAuxBus.IsReady() && InAuxBus.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InAuxBus.IsReady()))
	{
		auto* AuxBus = InAuxBus.Get();
		ResourceLoaderImpl->UnloadAuxBusAsync(MoveTemp(Promise), MoveTemp(AuxBus));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InAuxBus = MoveTemp(InAuxBus), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InAuxBus.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for Aux Bus load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for an Aux Bus to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for an Aux Bus to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* AuxBus = InAuxBus.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!AuxBus))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadAuxBusAsync(MoveTemp(Promise), MoveTemp(AuxBus));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedEventFuture FWwiseResourceLoader::LoadEventAsync(const FWwiseLocalizedEventCookedData& InEventCookedData,
                                                       const FWwiseLanguageCookedData* InLanguageOverride)
{
	FWwiseLoadedEventPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* EventNode = ResourceLoaderImpl->CreateEventNode(InEventCookedData, InLanguageOverride);
		if (UNLIKELY(!EventNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadEventAsync(MoveTemp(Promise), MoveTemp(EventNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadEventAsync(FWwiseLoadedEventFuture&& InEvent)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InEvent.IsReady() && InEvent.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InEvent.IsReady()))
	{
		auto* Event = InEvent.Get();
		ResourceLoaderImpl->UnloadEventAsync(MoveTemp(Promise), MoveTemp(Event));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InEvent = MoveTemp(InEvent), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InEvent.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for Event load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a Event to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a Event to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* Event = InEvent.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!Event))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadEventAsync(MoveTemp(Promise), MoveTemp(Event));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedExternalSourceFuture FWwiseResourceLoader::LoadExternalSourceAsync(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	FWwiseLoadedExternalSourcePromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* ExternalSourceNode = ResourceLoaderImpl->CreateExternalSourceNode(InExternalSourceCookedData);
		if (UNLIKELY(!ExternalSourceNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadExternalSourceAsync(MoveTemp(Promise), MoveTemp(ExternalSourceNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadExternalSourceAsync(FWwiseLoadedExternalSourceFuture&& InExternalSource)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InExternalSource.IsReady() && InExternalSource.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InExternalSource.IsReady()))
	{
		auto* ExternalSource = InExternalSource.Get();
		ResourceLoaderImpl->UnloadExternalSourceAsync(MoveTemp(Promise), MoveTemp(ExternalSource));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InExternalSource = MoveTemp(InExternalSource), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InExternalSource.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for External Source load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a External Source to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a External Source to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* ExternalSource = InExternalSource.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!ExternalSource))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadExternalSourceAsync(MoveTemp(Promise), MoveTemp(ExternalSource));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedGroupValueFuture FWwiseResourceLoader::LoadGroupValueAsync(const FWwiseGroupValueCookedData& InGroupValueCookedData)
{
	FWwiseLoadedGroupValuePromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* GroupValueNode = ResourceLoaderImpl->CreateGroupValueNode(InGroupValueCookedData);
		if (UNLIKELY(!GroupValueNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadGroupValueAsync(MoveTemp(Promise), MoveTemp(GroupValueNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadGroupValueAsync(FWwiseLoadedGroupValueFuture&& InGroupValue)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InGroupValue.IsReady() && InGroupValue.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InGroupValue.IsReady()))
	{
		auto* GroupValue = InGroupValue.Get();
		ResourceLoaderImpl->UnloadGroupValueAsync(MoveTemp(Promise), MoveTemp(GroupValue));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InGroupValue = MoveTemp(InGroupValue), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InGroupValue.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for Group Value load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a Group Value to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a Group Value to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* GroupValue = InGroupValue.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!GroupValue))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadGroupValueAsync(MoveTemp(Promise), MoveTemp(GroupValue));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedInitBankFuture FWwiseResourceLoader::LoadInitBankAsync(const FWwiseInitBankCookedData& InInitBankCookedData)
{
	FWwiseLoadedInitBankPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* InitBankNode = ResourceLoaderImpl->CreateInitBankNode(InInitBankCookedData);
		if (UNLIKELY(!InitBankNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadInitBankAsync(MoveTemp(Promise), MoveTemp(InitBankNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadInitBankAsync(FWwiseLoadedInitBankFuture&& InInitBank)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InInitBank.IsReady() && InInitBank.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InInitBank.IsReady()))
	{
		auto* InitBank = InInitBank.Get();
		ResourceLoaderImpl->UnloadInitBankAsync(MoveTemp(Promise), MoveTemp(InitBank));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InInitBank = MoveTemp(InInitBank), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InInitBank.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for Init Bank load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a Init Bank to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a Init Bank to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* InitBank = InInitBank.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!InitBank))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadInitBankAsync(MoveTemp(Promise), MoveTemp(InitBank));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedMediaFuture FWwiseResourceLoader::LoadMediaAsync(const FWwiseMediaCookedData& InMediaCookedData)
{
	FWwiseLoadedMediaPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* MediaNode = ResourceLoaderImpl->CreateMediaNode(InMediaCookedData);
		if (UNLIKELY(!MediaNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadMediaAsync(MoveTemp(Promise), MoveTemp(MediaNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadMediaAsync(FWwiseLoadedMediaFuture&& InMedia)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InMedia.IsReady() && InMedia.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InMedia.IsReady()))
	{
		auto* Media = InMedia.Get();
		ResourceLoaderImpl->UnloadMediaAsync(MoveTemp(Promise), MoveTemp(Media));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InMedia = MoveTemp(InMedia), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InMedia.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for Media load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a Media to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a Media to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* Media = InMedia.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!Media))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadMediaAsync(MoveTemp(Promise), MoveTemp(Media));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedShareSetFuture FWwiseResourceLoader::LoadShareSetAsync(
	const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	FWwiseLoadedShareSetPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* ShareSetNode = ResourceLoaderImpl->CreateShareSetNode(InShareSetCookedData, InLanguageOverride);
		if (UNLIKELY(!ShareSetNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadShareSetAsync(MoveTemp(Promise), MoveTemp(ShareSetNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadShareSetAsync(FWwiseLoadedShareSetFuture&& InShareSet)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InShareSet.IsReady() && InShareSet.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InShareSet.IsReady()))
	{
		auto* ShareSet = InShareSet.Get();
		ResourceLoaderImpl->UnloadShareSetAsync(MoveTemp(Promise), MoveTemp(ShareSet));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InShareSet = MoveTemp(InShareSet), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InShareSet.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for ShareSet load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a ShareSet to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a ShareSet to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* ShareSet = InShareSet.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!ShareSet))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadShareSetAsync(MoveTemp(Promise), MoveTemp(ShareSet));
			}
		});
	}

	return Future.Share();
}

FWwiseLoadedSoundBankFuture FWwiseResourceLoader::LoadSoundBankAsync(
	const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	FWwiseLoadedSoundBankPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue(nullptr);
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue(nullptr);
	}
	else
	{
		auto* SoundBankNode = ResourceLoaderImpl->CreateSoundBankNode(InSoundBankCookedData, InLanguageOverride);
		if (UNLIKELY(!SoundBankNode))
		{
			Promise.EmplaceValue(nullptr);
		}
		else
		{
			ResourceLoaderImpl->LoadSoundBankAsync(MoveTemp(Promise), MoveTemp(SoundBankNode));
		}
	}

	return Future.Share();
}

FWwiseResourceUnloadFuture FWwiseResourceLoader::UnloadSoundBankAsync(FWwiseLoadedSoundBankFuture&& InSoundBank)
{
	FWwiseResourceUnloadPromise Promise;
	auto Future = Promise.GetFuture();

	if (UNLIKELY(!ResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
		Promise.EmplaceValue();
	}
	else if (!IsEnabled())
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
		Promise.EmplaceValue();
	}
	else if (UNLIKELY(InSoundBank.IsReady() && InSoundBank.Get() == nullptr))
	{
		Promise.EmplaceValue();
	}
	else if (LIKELY(InSoundBank.IsReady()))
	{
		auto* SoundBank = InSoundBank.Get();
		ResourceLoaderImpl->UnloadSoundBankAsync(MoveTemp(Promise), MoveTemp(SoundBank));
	}
	else
	{
		AsyncTask(ResourceLoaderImpl->TaskThread, [this, InSoundBank = MoveTemp(InSoundBank), Promise = MoveTemp(Promise)]() mutable
		{
			{
				int WaitCount = 0;
				while (!InSoundBank.WaitFor(FTimespan::FromSeconds(1.0)))
				{
					if (IsEngineExitRequested())
					{
						UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Giving up on waiting for SoundBank load since we are exiting. Gave up on count [%d]"), WaitCount);
						Promise.EmplaceValue();
						return;
					}
					else
					{
						UE_CLOG(WaitCount != 10, LogWwiseResourceLoader, Verbose, TEXT("Waiting for a SoundBank to be fully loaded so we can unload it [%d]"), WaitCount);
						UE_CLOG(WaitCount == 10, LogWwiseResourceLoader, Warning, TEXT("Waited 10 seconds for a SoundBank to be loaded so we can unload it."));
						++WaitCount;
					}
				}
			}
			auto* SoundBank = InSoundBank.Get();

			if (UNLIKELY(!ResourceLoaderImpl))
			{
				UE_LOG(LogWwiseResourceLoader, Error, TEXT("No ResourceLoaderImpl"));
				Promise.EmplaceValue();
			}
			else if (!IsEnabled())
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("ResourceLoaderImpl is disabled"));
				Promise.EmplaceValue();
			}
			else if (UNLIKELY(!SoundBank))
			{
				Promise.EmplaceValue();
			}
			else
			{
				ResourceLoaderImpl->UnloadSoundBankAsync(MoveTemp(Promise), MoveTemp(SoundBank));
			}
		});
	}

	return Future.Share();
}

//
// Basic info
//

FWwiseSharedPlatformId FWwiseResourceLoader::SystemPlatform() const
{
	auto Result = FWwiseSharedPlatformId();
	Result.Platform = DefaultPlatform;
	return Result;
}

FWwiseLanguageCookedData FWwiseResourceLoader::SystemLanguage() const
{
	return DefaultLanguage;
}
