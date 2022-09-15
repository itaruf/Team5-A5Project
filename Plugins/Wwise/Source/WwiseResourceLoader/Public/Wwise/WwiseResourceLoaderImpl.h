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

#include "Wwise/CookedData/WwiseAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseEventCookedData.h"
#include "Wwise/CookedData/WwiseSharesetCookedData.h"
#include "Wwise/Loaded/WwiseLoadedAuxBus.h"
#include "Wwise/Loaded/WwiseLoadedSoundBank.h"
#include "Wwise/Loaded/WwiseLoadedEvent.h"
#include "Wwise/Loaded/WwiseLoadedExternalSource.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"
#include "Wwise/Loaded/WwiseLoadedInitBank.h"
#include "Wwise/Loaded/WwiseLoadedMedia.h"
#include "Wwise/Loaded/WwiseLoadedShareset.h"

#include "Wwise/WwiseSharedGroupValueKey.h"
#include "Wwise/WwiseSharedLanguageId.h"
#include "Wwise/WwiseSharedPlatformId.h"

#include "Wwise/Stats/ResourceLoader.h"

#include "WwiseResourceLoaderImpl.generated.h"

/**
 * @brief What reload strategy should be used for language changes
*/
UENUM()
enum EWwiseReloadLanguage
{
	/// Don't reload anything. The game is fully responsible to reload elements. This doesn't call
	/// any operation on the SoundEngine side, so everything will keep on working as usual.
	Manual,

	/// Reloads immediately without stopping anything. Game is responsible for stopping and restarting
	/// possibly affected sounds or else they might cause audible breaks. This is useful when some
	/// sounds can keep on playing, such as music and ambient sounds, while the dialogues are being
	/// internally reloaded.
	/// 
	/// Depending on the quantity of currently loaded localized banks, the operation can take a long time.
	/// 
	/// \warning Affected events needs to be restarted once the operation is done.
	Immediate,

	/// Stops all sounds first, unloads all the localized banks, and reloads the new language. This will
	/// cause an audible break while the operation is done.
	/// 
	/// Depending on the quantity of currently loaded localized banks, the operation can take a long time.
	/// 
	/// \warning Affected events needs to be restarted once the operation is done.
	Safe
};

struct WWISERESOURCELOADER_API FWwiseSwitchContainerLeafGroupValueUsageCount
{
	/**
	 * @brief SwitchContainer Leaf this structure represents.
	*/
	const FWwiseSwitchContainerLeafCookedData& Key;

	/**
	 * @brief Number of GroupValue this particular key that are already loaded for this key.
	*/
	TSet<FWwiseGroupValueCookedData> LoadedGroupValues;

	/**
	 * @brief True if the media represented by the Key was successfully loaded.
	*/
	bool bLoaded;

	FWwiseSwitchContainerLeafGroupValueUsageCount(const FWwiseSwitchContainerLeafCookedData& InKey) :
		Key(InKey),
		bLoaded(false)
	{}

	bool HaveAllKeys() const
	{
		check(Key.GroupValueSet.Num() >= LoadedGroupValues.Num());
		return Key.GroupValueSet.Num() == LoadedGroupValues.Num();
	}
};

struct WWISERESOURCELOADER_API FWwiseLoadedGroupValueInfo
{
	/**
	 * @brief GroupValue key this structure represents.
	*/
	FWwiseSharedGroupValueKey Key;

	/**
	 * @brief Number of times this particular GroupValue got loaded in the currently loaded maps.
	 * 
	 * Any value higher than 0 means there's a chance the Leaves might be required.
	*/
	int LoadCount;

	/**
	 * @brief Leaves that uses this particular GroupValue.
	 * 
	 * @note The ownership of this pointer is uniquely created and discarded during SwitchContainerLeaf loading and unloading.
	*/
	TSet<FWwiseSwitchContainerLeafGroupValueUsageCount*> Leaves;

	FWwiseLoadedGroupValueInfo(const FWwiseSharedGroupValueKey& InKey) :
		Key(InKey),
		LoadCount(0),
		Leaves()
	{}

	bool ShouldBeLoaded() const
	{
		check(LoadCount >= 0);

		return LoadCount > 0;
	}

	bool operator ==(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key == InRhs.Key;
	}

	bool operator !=(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key != InRhs.Key;
	}

	bool operator <(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key < InRhs.Key;
	}

	bool operator <=(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key <= InRhs.Key;
	}

	bool operator >(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key > InRhs.Key;
	}

	bool operator >=(const FWwiseLoadedGroupValueInfo& InRhs) const
	{
		return Key >= InRhs.Key;
	}

	bool operator ==(const FWwiseSharedGroupValueKey& InRhs) const
	{
		return Key == InRhs;
	}

	bool operator !=(const FWwiseSharedGroupValueKey& InRhs) const
	{
		return Key != InRhs;
	}
};
inline uint32 GetTypeHash(const FWwiseLoadedGroupValueInfo& InValue)
{
	return GetTypeHash(InValue.Key);
}

UCLASS()
class WWISERESOURCELOADER_API UWwiseResourceLoaderImpl : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Currently targeted platform for this runtime
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise")
	FWwiseSharedPlatformId CurrentPlatform;

	/**
	 * @brief Currently targeted language for this runtime
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise")
	FWwiseLanguageCookedData CurrentLanguage;

	/**
	 * @brief Location in the staged product where the SoundBank medias are found
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise")
	FString StagePath;

#if WITH_EDITORONLY_DATA
	/**
	 * @brief Location where the Wwise Generated SoundBanks product is found on disk relative to the project
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise")
	FDirectoryPath GeneratedSoundBanksPath;
#endif

	FWwiseLoadedAuxBusList LoadedAuxBusList;
	FWwiseLoadedSoundBankList LoadedSoundBankList;
	FWwiseLoadedEventList LoadedEventList;
	FWwiseLoadedExternalSourceList LoadedExternalSourceList;
	FWwiseLoadedGroupValueList LoadedGroupValueList;
	FWwiseLoadedInitBankList LoadedInitBankList;
	FWwiseLoadedMediaList LoadedMediaList;
	FWwiseLoadedSharesetList LoadedSharesetList;

	TSet<FWwiseLoadedGroupValueInfo> LoadedGroupValueInfo;
	TMap<uint32, TArray<FWwiseLoadedGroupValueListNode*>> LoadedEventRequiredGroupValueNodes;

	UWwiseResourceLoaderImpl();

	FString GetUnrealExternalSourcePath() const;
	FString GetUnrealPath() const;
	FString GetUnrealPath(const FString& InPath) const;

	FString GetUnrealStagePath(const FString& InPath) const;
#if WITH_EDITORONLY_DATA
	FString GetUnrealGeneratedSoundBanksPath(const FString& InPath) const;
#endif

	bool DefaultLoadSoundBankOperation(const FWwiseSoundBankCookedData& InSoundBank);
	bool DefaultUnloadSoundBankOperation(const FWwiseSoundBankCookedData& InSoundBank);
	bool DefaultLoadMediaOperation(const FWwiseMediaCookedData& InMedia);
	bool DefaultUnloadMediaOperation(const FWwiseMediaCookedData& InMedia);
	bool DefaultLoadExternalSourceOperation(const FWwiseExternalSourceCookedData& InExternalSource);
	bool DefaultUnloadExternalSourceOperation(const FWwiseExternalSourceCookedData& InExternalSource);

	template<typename MapValue>
	inline const FWwiseLanguageCookedData* GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FString& InDebugName) const;

	virtual void SetLanguage(const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage);
	void SetPlatform(const FWwiseSharedPlatformId& InPlatform);

	virtual FWwiseLoadedAuxBusListNode* LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual bool LoadAuxBusResources(const FWwiseAuxBusCookedData& InCookedData);
	virtual void UnloadAuxBus(FWwiseLoadedAuxBusListNode* InAuxBusListNode);
	virtual void UnloadAuxBusResources(const FWwiseAuxBusCookedData& InCookedData);

	virtual FWwiseLoadedEventListNode* LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual bool LoadEventResources(const FWwiseEventCookedData& InCookedData);
	virtual bool LoadEventSwitchContainerResources(const FWwiseEventCookedData& InCookedData);
	virtual void UnloadEvent(FWwiseLoadedEventListNode* InEventListNode);
	virtual void UnloadEventResources(const FWwiseEventCookedData& InCookedData);
	virtual bool UnloadEventSwitchContainerResources(const FWwiseEventCookedData& InCookedData);

	virtual FWwiseLoadedExternalSourceListNode* LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	virtual bool LoadExternalSourceResources(const FWwiseExternalSourceCookedData& InCookedData);
	virtual void UnloadExternalSource(FWwiseLoadedExternalSourceListNode* InMediaListNode);
	virtual void UnloadExternalSourceResources(const FWwiseExternalSourceCookedData& InCookedData);

	virtual FWwiseLoadedGroupValueListNode* LoadGroupValue(const FWwiseGroupValueCookedData& InSwitchCookedData);
	virtual bool LoadGroupValueResources(const FWwiseGroupValueCookedData& InCookedData);
	virtual void UnloadGroupValue(FWwiseLoadedGroupValueListNode* InSwitchListNode);
	virtual void UnloadGroupValueResources(const FWwiseGroupValueCookedData& InCookedData);

	virtual FWwiseLoadedInitBankListNode* LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData);
	virtual bool LoadInitBankResources(const FWwiseInitBankCookedData& InCookedData);
	virtual void UnloadInitBank(FWwiseLoadedInitBankListNode* InInitBankListNode);
	virtual void UnloadInitBankResources(const FWwiseInitBankCookedData& InCookedData);

	virtual FWwiseLoadedMediaListNode* LoadMedia(const FWwiseMediaCookedData& InMediaCookedData);
	virtual bool LoadMediaResources(const FWwiseMediaCookedData& InCookedData);
	virtual void UnloadMedia(FWwiseLoadedMediaListNode* InMediaListNode);
	virtual void UnloadMediaResources(const FWwiseMediaCookedData& InCookedData);

	virtual FWwiseLoadedSharesetListNode* LoadShareset(const FWwiseLocalizedSharesetCookedData& InSharesetCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual bool LoadSharesetResources(const FWwiseSharesetCookedData& InCookedData);
	virtual void UnloadShareset(FWwiseLoadedSharesetListNode* InSharesetListNode);
	virtual void UnloadSharesetResources(const FWwiseSharesetCookedData& InCookedData);

	virtual FWwiseLoadedSoundBankListNode* LoadSoundBank(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual bool LoadSoundBankResources(const FWwiseSoundBankCookedData& InCookedData);
	virtual void UnloadSoundBank(FWwiseLoadedSoundBankListNode* InSoundBankListNode);
	virtual void UnloadSoundBankResources(const FWwiseSoundBankCookedData& InCookedData);

	virtual bool LoadSwitchContainerLeafResources(const FWwiseSwitchContainerLeafCookedData& InCookedData);
	virtual bool UnloadSwitchContainerLeafResources(const FWwiseSwitchContainerLeafCookedData& InCookedData);
};

template<typename MapValue>
inline const FWwiseLanguageCookedData* UWwiseResourceLoaderImpl::GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FString& InDebugName) const
{
	if (InLanguageOverride)
	{
		if (Map.Find(*InLanguageOverride))
		{
			return InLanguageOverride;
		}
		UE_LOG(LogWwiseResourceLoader, Log, TEXT("GetLanguageMapKey: Could not find overridden language %s while processing asset %s. Defaulting to language %s"),
			*InLanguageOverride->GetLanguageName(), *InDebugName, *CurrentLanguage.GetLanguageName());
	}

	if (LIKELY(Map.Contains(FWwiseLanguageCookedData::Sfx)))
	{
		return &FWwiseLanguageCookedData::Sfx;
	}

	if (Map.Find(CurrentLanguage))
	{
		return &CurrentLanguage;
	}

	UE_LOG(LogWwiseResourceLoader, Warning, TEXT("GetLanguageMapKey: Could not find language %s while processing asset %s."),
		*CurrentLanguage.GetLanguageName(), *InDebugName);
	return nullptr;
}

