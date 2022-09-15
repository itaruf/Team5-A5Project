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

#include "Wwise/WwiseResourceLoader.h"

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"

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

FWwiseResourceLoaderImplScopeLock::FWwiseResourceLoaderImplScopeLock(const UWwiseResourceLoader* InRuntime) :
	FRWScopeLock(const_cast<FRWLock&>(InRuntime->Lock), SLT_ReadOnly),
	ResourceLoaderImpl(*InRuntime->LockedResourceLoaderImpl)
{
}

FWwiseResourceLoaderImplWriteScopeLock::FWwiseResourceLoaderImplWriteScopeLock(UWwiseResourceLoader* InRuntime) :
	FRWScopeLock(InRuntime->Lock, SLT_Write),
	ResourceLoaderImpl(*InRuntime->LockedResourceLoaderImpl)
{
}

UWwiseResourceLoader::UWwiseResourceLoader() :
	LockedResourceLoaderImpl(InstantiateResourceLoaderImpl())
{
	LockedResourceLoaderImpl->CurrentLanguage = SystemLanguage();
	LockedResourceLoaderImpl->CurrentPlatform = SystemPlatform();
}

void UWwiseResourceLoader::SetLanguage(FWwiseLanguageCookedData InLanguage, EWwiseReloadLanguage InReloadLanguage)
{
	FWwiseResourceLoaderImplWriteScopeLock(this)->SetLanguage(InLanguage, InReloadLanguage);
}

void UWwiseResourceLoader::SetPlatform(const FWwiseSharedPlatformId& InPlatform)
{
	FWwiseResourceLoaderImplWriteScopeLock(this)->SetPlatform(InPlatform);
}

FWwiseLanguageCookedData UWwiseResourceLoader::GetCurrentLanguage() const
{
	return FWwiseResourceLoaderImplScopeLock(this)->CurrentLanguage;
}

FWwiseSharedPlatformId UWwiseResourceLoader::GetCurrentPlatform() const
{
	return FWwiseResourceLoaderImplScopeLock(this)->CurrentPlatform;
}

FString UWwiseResourceLoader::GetUnrealPath(const FString& InPath) const
{
	return FWwiseResourceLoaderImplScopeLock(this)->GetUnrealPath(InPath);
}

FString UWwiseResourceLoader::GetUnrealExternalSourcePath() const
{
	return FWwiseResourceLoaderImplScopeLock(this)->GetUnrealExternalSourcePath();
}

FString UWwiseResourceLoader::GetUnrealStagePath(const FString& InPath) const
{
	return FWwiseResourceLoaderImplScopeLock(this)->GetUnrealStagePath(InPath);
}

#if WITH_EDITORONLY_DATA
FString UWwiseResourceLoader::GetUnrealGeneratedSoundBanksPath(const FString& InPath) const
{
	return FWwiseResourceLoaderImplScopeLock(this)->GetUnrealGeneratedSoundBanksPath(InPath);
}
#endif

//
// User-facing loading and unloading operations
//

FWwiseLoadedAuxBusListNode* UWwiseResourceLoader::LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadAuxBus(InAuxBusCookedData, InLanguageOverride);
}

void UWwiseResourceLoader::UnloadAuxBus(FWwiseLoadedAuxBusListNode* InAuxBusListNode)
{
	if (UNLIKELY(!InAuxBusListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadAuxBus(InAuxBusListNode);
}

FWwiseLoadedEventListNode* UWwiseResourceLoader::LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadEvent(InEventCookedData, InLanguageOverride);
}

void UWwiseResourceLoader::UnloadEvent(FWwiseLoadedEventListNode* InEventListNode)
{
	if (UNLIKELY(!InEventListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadEvent(InEventListNode);
}

FWwiseLoadedExternalSourceListNode* UWwiseResourceLoader::LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadExternalSource(InExternalSourceCookedData);
}

void UWwiseResourceLoader::UnloadExternalSource(FWwiseLoadedExternalSourceListNode* InExternalSourceListNode)
{
	if (UNLIKELY(!InExternalSourceListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadExternalSource(InExternalSourceListNode);
}

FWwiseLoadedGroupValueListNode* UWwiseResourceLoader::LoadGroupValue(const FWwiseGroupValueCookedData& InSwitchCookedData)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadGroupValue(InSwitchCookedData);
}

void UWwiseResourceLoader::UnloadGroupValue(FWwiseLoadedGroupValueListNode* InSwitchListNode)
{
	if (UNLIKELY(!InSwitchListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadGroupValue(InSwitchListNode);
}

FWwiseLoadedInitBankListNode* UWwiseResourceLoader::LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadInitBank(InInitBankCookedData);
}

void UWwiseResourceLoader::UnloadInitBank(FWwiseLoadedInitBankListNode* InInitBankListNode)
{
	if (UNLIKELY(!InInitBankListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadInitBank(InInitBankListNode);
}

FWwiseLoadedMediaListNode* UWwiseResourceLoader::LoadMedia(const FWwiseMediaCookedData& InMediaCookedData)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadMedia(InMediaCookedData);
}

void UWwiseResourceLoader::UnloadMedia(FWwiseLoadedMediaListNode* InMediaListNode)
{
	if (UNLIKELY(!InMediaListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadMedia(InMediaListNode);
}

FWwiseLoadedSharesetListNode* UWwiseResourceLoader::LoadShareset(const FWwiseLocalizedSharesetCookedData& InSharesetCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadShareset(InSharesetCookedData, InLanguageOverride);
}

void UWwiseResourceLoader::UnloadShareset(FWwiseLoadedSharesetListNode* InSharesetListNode)
{
	if (UNLIKELY(!InSharesetListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadShareset(InSharesetListNode);
}

FWwiseLoadedSoundBankListNode* UWwiseResourceLoader::LoadSoundBank(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	return FWwiseResourceLoaderImplWriteScopeLock(this)->LoadSoundBank(InSoundBankCookedData, InLanguageOverride);
}

void UWwiseResourceLoader::UnloadSoundBank(FWwiseLoadedSoundBankListNode* InSoundBankListNode)
{
	if (UNLIKELY(!InSoundBankListNode))
	{
		return;
	}

	return FWwiseResourceLoaderImplWriteScopeLock(this)->UnloadSoundBank(InSoundBankListNode);
}

//
// Basic info
//

FWwiseSharedPlatformId UWwiseResourceLoader::SystemPlatform() const
{
	auto Result = FWwiseSharedPlatformId();
	Result.Platform = DefaultPlatform;
	return Result;
}

FWwiseLanguageCookedData UWwiseResourceLoader::SystemLanguage() const
{
	return DefaultLanguage;
}
