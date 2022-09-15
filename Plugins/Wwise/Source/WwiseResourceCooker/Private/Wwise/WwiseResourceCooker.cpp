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

#include "Wwise/WwiseResourceCooker.h"

#include "Wwise/Stats/ResourceCooker.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseCookingCache.h"
#include "Wwise/Stats/ResourceCooker.h"

#include "Async/Async.h"

void UWwiseResourceCooker::CookAuxBus(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookAuxBus: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->AuxBusCache.Find(InInfo))
	{
		CookLocalizedAuxBusToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseLocalizedAuxBusCookedData CookedData;
		if (UNLIKELY(!GetAuxBusCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->AuxBusCache.Add(InInfo, CookedData);
		CookLocalizedAuxBusToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookEvent(const FWwiseEventInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookEvent: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->EventCache.Find(InInfo))
	{
		CookLocalizedEventToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseLocalizedEventCookedData CookedData;
		if (UNLIKELY(!GetEventCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->EventCache.Add(InInfo, CookedData);
		CookLocalizedEventToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookExternalSource(uint32 InCookie, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookExternalSource: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->ExternalSourceCache.Find(InCookie))
	{
		CookExternalSourceToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseExternalSourceCookedData CookedData;
		if (UNLIKELY(!GetExternalSourceCookedData(CookedData, InCookie)))
		{
			return;
		}

		CookingCache->ExternalSourceCache.Add(InCookie, CookedData);
		CookExternalSourceToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookInitBank(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookInitBank: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->InitBankCache.Find(InInfo))
	{
		CookInitBankToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseInitBankCookedData CookedData;
		if (UNLIKELY(!GetInitBankCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->InitBankCache.Add(InInfo, CookedData);
		CookInitBankToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookMedia(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookMedia: No CookingCache."));
		return;
	}

	FWwiseDatabaseMediaIdKey MediaKey;
	MediaKey.MediaId = InInfo.AssetShortId;
	MediaKey.SoundBankId = InInfo.HardCodedSoundBankShortId;

	if (const auto* CachedCookedData = CookingCache->MediaCache.Find(MediaKey))
	{
		CookMediaToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseMediaCookedData CookedData;
		if (UNLIKELY(!GetMediaCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->MediaCache.Add(MediaKey, CookedData);
		CookMediaToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookShareset(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookShareset: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->SharesetCache.Find(InInfo))
	{
		CookLocalizedSharesetToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseLocalizedSharesetCookedData CookedData;
		if (UNLIKELY(!GetSharesetCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->SharesetCache.Add(InInfo, CookedData);
		CookLocalizedSharesetToSandbox(CookedData, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookSoundBank(const FWwiseAssetInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile)
{
	auto* CookingCache = GetCookingCache();
	if (UNLIKELY(!CookingCache))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CookSoundBank: No CookingCache."));
		return;
	}

	if (const auto* CachedCookedData = CookingCache->SoundBankCache.Find(InInfo))
	{
		CookLocalizedSoundBankToSandbox(*CachedCookedData, WriteAdditionalFile);
	}
	else
	{
		FWwiseLocalizedSoundBankCookedData CookedData;
		if (UNLIKELY(!GetSoundBankCookedData(CookedData, InInfo)))
		{
			return;
		}

		CookingCache->SoundBankCache.Add(InInfo, CookedData);
		CookLocalizedSoundBankToSandbox(CookedData, WriteAdditionalFile);
	}
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetAcousticTextureCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->AcousticTextureCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetAcousticTextureCookedData(OutCookedData, InInfo)))
	{
		CookingCache->AcousticTextureCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetAuxBusCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->AuxBusCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetAuxBusCookedData(OutCookedData, InInfo)))
	{
		CookingCache->AuxBusCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetEventCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->EventCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetEventCookedData(OutCookedData, InInfo)))
	{
		CookingCache->EventCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetExternalSourceCookedData(OutCookedData, InCookie);
	}

	if (const auto* CachedCookedData = CookingCache->ExternalSourceCache.Find(InCookie))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetExternalSourceCookedData(OutCookedData, InCookie)))
	{
		CookingCache->ExternalSourceCache.Add(InCookie, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetGameParameterCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->GameParameterCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetGameParameterCookedData(OutCookedData, InInfo)))
	{
		CookingCache->GameParameterCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo, EWwiseGroupType InGroupType)
{
	auto* CookingCache = GetCookingCache();
	if (InGroupType == EWwiseGroupType::State)
	{
		if (!CookingCache)
		{
			return GetStateCookedData(OutCookedData, InInfo);
		}

		if (const auto* CachedCookedData = CookingCache->StateCache.Find(InInfo))
		{
			OutCookedData = *CachedCookedData;
			return true;
		}
		else if (LIKELY(GetStateCookedData(OutCookedData, InInfo)))
		{
			CookingCache->StateCache.Add(InInfo, OutCookedData);
			return true;
		}
	}
	else if (InGroupType == EWwiseGroupType::Switch)
	{
		if (!CookingCache)
		{
			return GetSwitchCookedData(OutCookedData, InInfo);
		}

		if (const auto* CachedCookedData = CookingCache->SwitchCache.Find(InInfo))
		{
			OutCookedData = *CachedCookedData;
			return true;
		}
		else if (LIKELY(GetSwitchCookedData(OutCookedData, InInfo)))
		{
			CookingCache->SwitchCache.Add(InInfo, OutCookedData);
			return true;
		}
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetInitBankCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->InitBankCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetInitBankCookedData(OutCookedData, InInfo)))
	{
		CookingCache->InitBankCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetMediaCookedData(OutCookedData, InInfo);
	}

	FWwiseDatabaseMediaIdKey MediaIdKey;
	MediaIdKey.MediaId = InInfo.AssetShortId;
	MediaIdKey.SoundBankId = InInfo.HardCodedSoundBankShortId;

	if (const auto* CachedCookedData = CookingCache->MediaCache.Find(MediaIdKey))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetMediaCookedData(OutCookedData, InInfo)))
	{
		CookingCache->MediaCache.Add(MediaIdKey, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseLocalizedSharesetCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetSharesetCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->SharesetCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetSharesetCookedData(OutCookedData, InInfo)))
	{
		CookingCache->SharesetCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetSoundBankCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->SoundBankCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetSoundBankCookedData(OutCookedData, InInfo)))
	{
		CookingCache->SoundBankCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

bool UWwiseResourceCooker::PrepareCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseAssetInfo& InInfo)
{
	auto* CookingCache = GetCookingCache();
	if (!CookingCache)
	{
		return GetTriggerCookedData(OutCookedData, InInfo);
	}

	if (const auto* CachedCookedData = CookingCache->TriggerCache.Find(InInfo))
	{
		OutCookedData = *CachedCookedData;
		return true;
	}
	else if (LIKELY(GetTriggerCookedData(OutCookedData, InInfo)))
	{
		CookingCache->TriggerCache.Add(InInfo, OutCookedData);
		return true;
	}
	return false;
}

void UWwiseResourceCooker::SetSandboxRootPath(const TCHAR* InPackageFilename)
{
	if (!SandboxRootPath.IsEmpty())
	{
		return;
	}

	auto SandboxPath = FPaths::GetPath(InPackageFilename); // Remove Filename.uasset
	while (!SandboxPath.IsEmpty() && FPaths::GetCleanFilename(SandboxPath) != TEXT("Content"))
	{
		SandboxPath = FPaths::GetPath(SandboxPath);
	}

	UE_LOG(LogWwiseResourceCooker, Display, TEXT("SetSandboxRootPath: Updating Sandbox Root Path: %s"), *SandboxPath);

	SandboxRootPath = SandboxPath;
}

UWwiseResourceLoader* UWwiseResourceCooker::GetResourceLoader()
{
	if (auto* ProjectDatabase = GetProjectDatabase())
	{
		return ProjectDatabase->GetResourceLoader();
	}
	else
	{
		return UWwiseResourceLoader::Get();
	}
}

const UWwiseResourceLoader* UWwiseResourceCooker::GetResourceLoader() const
{
	if (const auto* ProjectDatabase = GetProjectDatabase())
	{
		return ProjectDatabase->GetResourceLoader();
	}
	else
	{
		return UWwiseResourceLoader::Get();
	}
}

FWwiseSharedLanguageId UWwiseResourceCooker::GetCurrentLanguage() const
{
	if (const auto* ProjectDatabase = GetProjectDatabase())
	{
		return ProjectDatabase->GetCurrentLanguage();
	}
	return {};
}

FWwiseSharedPlatformId UWwiseResourceCooker::GetCurrentPlatform() const
{
	if (const auto* ProjectDatabase = GetProjectDatabase())
	{
		return ProjectDatabase->GetCurrentPlatform();
	}
	return {};
}

void UWwiseResourceCooker::CookLocalizedAuxBusToSandbox(const FWwiseLocalizedAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	for (const auto& AuxBus : InCookedData.AuxBusLanguageMap)
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking AuxBus %s in %s %" PRIu32), *InCookedData.DebugName, *AuxBus.Key.GetLanguageName(), (uint32)AuxBus.Key.GetLanguageId());
		CookAuxBusToSandbox(AuxBus.Value, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookLocalizedEventToSandbox(const FWwiseLocalizedEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	for (const auto& Event : InCookedData.EventLanguageMap)
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Event %s in %s %" PRIu32), *InCookedData.DebugName, *Event.Key.GetLanguageName(), (uint32)Event.Key.GetLanguageId());
		CookEventToSandbox(Event.Value, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookLocalizedSharesetToSandbox(const FWwiseLocalizedSharesetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	for (const auto& Shareset : InCookedData.SharesetLanguageMap)
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Shareset %s in %s %" PRIu32), *InCookedData.DebugName, *Shareset.Key.GetLanguageName(), (uint32)Shareset.Key.GetLanguageId());
		CookSharesetToSandbox(Shareset.Value, WriteAdditionalFile);
	}
}

void UWwiseResourceCooker::CookLocalizedSoundBankToSandbox(const FWwiseLocalizedSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	for (const auto& SoundBank : InCookedData.SoundBankLanguageMap)
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking SoundBank %s in %s %" PRIu32), *InCookedData.DebugName, *SoundBank.Key.GetLanguageName(), (uint32)SoundBank.Key.GetLanguageId());
		CookSoundBankToSandbox(SoundBank.Value, WriteAdditionalFile);
	}
}
