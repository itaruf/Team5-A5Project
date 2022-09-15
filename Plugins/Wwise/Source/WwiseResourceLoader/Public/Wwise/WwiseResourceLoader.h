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

#include "Wwise/WwiseResourceLoaderImpl.h"
#include "Wwise/WwiseResourceLoaderModule.h"

#include "WwiseResourceLoader.generated.h"

class UWwiseResourceLoader;
class WWISERESOURCELOADER_API FWwiseResourceLoaderImplScopeLock : public FRWScopeLock
{
public:
	FWwiseResourceLoaderImplScopeLock(const UWwiseResourceLoader* InRuntime);

	const UWwiseResourceLoaderImpl& operator*() const
	{
		return ResourceLoaderImpl;
	}

	const UWwiseResourceLoaderImpl* operator->() const
	{
		return &ResourceLoaderImpl;
	}

private:
	const UWwiseResourceLoaderImpl& ResourceLoaderImpl;
	UE_NONCOPYABLE(FWwiseResourceLoaderImplScopeLock);
};

class WWISERESOURCELOADER_API FWwiseResourceLoaderImplWriteScopeLock : public FRWScopeLock
{
public:
	FWwiseResourceLoaderImplWriteScopeLock(UWwiseResourceLoader* InRuntime);

	UWwiseResourceLoaderImpl& operator*()
	{
		return ResourceLoaderImpl;
	}

	UWwiseResourceLoaderImpl* operator->()
	{
		return &ResourceLoaderImpl;
	}

private:
	UWwiseResourceLoaderImpl& ResourceLoaderImpl;
	UE_NONCOPYABLE(FWwiseResourceLoaderImplWriteScopeLock);
};

/**
 * @brief Operations available to manage and handle Wwise SoundBanks in Unreal.
*/
UCLASS()
class WWISERESOURCELOADER_API UWwiseResourceLoader : public UEngineSubsystem
{
	GENERATED_BODY()

	friend class FWwiseResourceLoaderImplScopeLock;
	friend class FWwiseResourceLoaderImplWriteScopeLock;

public:
	inline static UWwiseResourceLoader* Get()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->GetResourceLoader();
		}
		return nullptr;
	}
	inline static UWwiseResourceLoaderImpl* InstantiateResourceLoaderImpl()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->InstantiateResourceLoaderImpl();
		}
		return nullptr;
	}

	UWwiseResourceLoader();

	//
	// User-facing operations
	//

	UFUNCTION()
	FWwiseLanguageCookedData GetCurrentLanguage() const;

	UFUNCTION()
	FWwiseSharedPlatformId GetCurrentPlatform() const;

	/**
	 * @brief Returns the actual Unreal file path needed in order to retrieve this particular Wwise Path.
	 * 
	 * This method acts differently depending on usage in ResourceLoaderImpl or Editor. In Editor, this will return
	 * the full path to the Generated SoundBanks folder. In a packaged game, this will return the full
	 * path to the staged file.
	 * 
	 * @param WwisePath Requested file path, as found in SoundBanksInfo.
	 * @return The corresponding Unreal path.
	*/
	UFUNCTION(BlueprintPure)
	FString GetUnrealPath(const FString& InPath) const;
	FString GetUnrealExternalSourcePath() const;

	FString GetUnrealStagePath(const FString& InPath) const;
#if WITH_EDITORONLY_DATA
	FString GetUnrealGeneratedSoundBanksPath(const FString& InPath) const;
#endif

	/**
	 * @brief Sets the language for the current runtime, optionally reloading all affected assets immediately
	 * @param LanguageId The current language being processed, or 0 if none
	 * @param ReloadLanguage What reload strategy should be used for language changes 
	*/
	UFUNCTION()
	void SetLanguage(FWwiseLanguageCookedData InLanguage, EWwiseReloadLanguage InReloadLanguage);

	void SetPlatform(const FWwiseSharedPlatformId& InPlatform);


	FWwiseLoadedAuxBusListNode* LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	void UnloadAuxBus(FWwiseLoadedAuxBusListNode* InAuxBusListNode);

	FWwiseLoadedEventListNode* LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	void UnloadEvent(FWwiseLoadedEventListNode* InEventListNode);

	FWwiseLoadedExternalSourceListNode* LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	void UnloadExternalSource(FWwiseLoadedExternalSourceListNode* InMediaListNode);

	FWwiseLoadedGroupValueListNode* LoadGroupValue(const FWwiseGroupValueCookedData& InSwitchCookedData);
	void UnloadGroupValue(FWwiseLoadedGroupValueListNode* InSwitchListNode);

	FWwiseLoadedInitBankListNode* LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData);
	void UnloadInitBank(FWwiseLoadedInitBankListNode* InInitBankListNode);

	FWwiseLoadedMediaListNode* LoadMedia(const FWwiseMediaCookedData& InMediaCookedData);
	void UnloadMedia(FWwiseLoadedMediaListNode* InMediaListNode);

	FWwiseLoadedSharesetListNode* LoadShareset(const FWwiseLocalizedSharesetCookedData& InSharesetCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	void UnloadShareset(FWwiseLoadedSharesetListNode* InSharesetListNode);

	FWwiseLoadedSoundBankListNode* LoadSoundBank(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	void UnloadSoundBank(FWwiseLoadedSoundBankListNode* InSoundBankListNode);

	UFUNCTION(BlueprintPure)
	virtual FWwiseSharedPlatformId SystemPlatform() const;

	UFUNCTION(BlueprintPure)
	virtual FWwiseLanguageCookedData SystemLanguage() const;

	UPROPERTY()
	UWwiseResourceLoaderImpl* LockedResourceLoaderImpl;

protected:
	FRWLock Lock;
};
