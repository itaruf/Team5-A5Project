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

#pragma once

#include "WwiseResourceLoaderModule.h"
#include "Wwise/WwiseExecutionQueue.h"
#include "Wwise/CookedData/WwiseAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseEventCookedData.h"
#include "Wwise/CookedData/WwiseShareSetCookedData.h"
#include "Wwise/Loaded/WwiseLoadedAuxBus.h"
#include "Wwise/Loaded/WwiseLoadedSoundBank.h"
#include "Wwise/Loaded/WwiseLoadedEvent.h"
#include "Wwise/Loaded/WwiseLoadedExternalSource.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"
#include "Wwise/Loaded/WwiseLoadedInitBank.h"
#include "Wwise/Loaded/WwiseLoadedMedia.h"
#include "Wwise/Loaded/WwiseLoadedShareSet.h"

#include "Wwise/WwiseSharedGroupValueKey.h"
#include "Wwise/WwiseSharedLanguageId.h"
#include "Wwise/WwiseSharedPlatformId.h"

#include "Wwise/Stats/ResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Engine/EngineTypes.h"
#include "UObject/SoftObjectPath.h"
#endif

#include "WwiseResourceLoaderImpl.generated.h"

using FWwiseResourceUnloadPromise = TPromise<void>;
using FWwiseResourceUnloadFuture = TSharedFuture<void>;

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

/**
 * @brief Whether the WwiseResourceLoader is allowed to load/unload assets
*/
enum class EWwiseResourceLoaderState
{
	/// Do not allow the WwiseResourceLoader to load/unload assets
	AlwaysDisabled,
	/// Allow the WwiseResourceLoader to load/unload assets
	Enabled,
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
	 * @brief Resources represented by the Key that were successfully loaded.
	*/
	struct FLoadedData
	{
		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseExternalSourceCookedData*> LoadedExternalSources;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;
		bool IsLoaded() const { return LoadedSoundBanks.Num() > 0 || LoadedExternalSources.Num() > 0 || LoadedMedia.Num() > 0; }
	} LoadedData;

	FWwiseSwitchContainerLeafGroupValueUsageCount(const FWwiseSwitchContainerLeafCookedData& InKey) :
		Key(InKey)
	{}

	bool HaveAllKeys() const
	{
		check(Key.GroupValueSet.Num() >= LoadedGroupValues.Num());
		return Key.GroupValueSet.Num() == LoadedGroupValues.Num();
	}
};

struct WWISERESOURCELOADER_API FWwiseSwitchContainerLoadedGroupValueInfo
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

	FWwiseSwitchContainerLoadedGroupValueInfo(const FWwiseSharedGroupValueKey& InKey) :
		Key(InKey),
		LoadCount(0),
		Leaves()
	{}

	bool ShouldBeLoaded() const
	{
		check(LoadCount >= 0);

		return LoadCount > 0;
	}

	bool operator ==(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key == InRhs.Key;
	}

	bool operator !=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key != InRhs.Key;
	}

	bool operator <(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key < InRhs.Key;
	}

	bool operator <=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key <= InRhs.Key;
	}

	bool operator >(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key > InRhs.Key;
	}

	bool operator >=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
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
inline uint32 GetTypeHash(const FWwiseSwitchContainerLoadedGroupValueInfo& InValue)
{
	return GetTypeHash(InValue.Key);
}

class WWISERESOURCELOADER_API FWwiseResourceLoaderImpl
{
public:
	static FWwiseResourceLoaderImpl* Instantiate()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->InstantiateResourceLoaderImpl();
		}
		return nullptr;
	}

	EWwiseResourceLoaderState WwiseResourceLoaderState = EWwiseResourceLoaderState::Enabled;

	/**
	 * @brief Currently targeted platform for this runtime
	*/
	FWwiseSharedPlatformId CurrentPlatform;

	/**
	 * @brief Currently targeted language for this runtime
	*/
	FWwiseLanguageCookedData CurrentLanguage;

	/**
	 * @brief Location in the staged product where the SoundBank medias are found
	*/
	FString StagePath;

#if WITH_EDITORONLY_DATA
	/**
	 * @brief Location where the Wwise Generated SoundBanks product is found on disk relative to the project
	*/
	FDirectoryPath GeneratedSoundBanksPath;
#endif

	ENamedThreads::Type TaskThread = ENamedThreads::AnyThread;

	FWwiseResourceLoaderImpl();
	virtual ~FWwiseResourceLoaderImpl() {}

	FName GetUnrealExternalSourcePath() const;
	FString GetUnrealPath() const;
	FString GetUnrealPath(const FName& InPath) const { return GetUnrealPath(InPath.ToString()); }
	FString GetUnrealPath(const FString& InPath) const;

	FString GetUnrealStagePath(const FName& InPath) const;
	FString GetUnrealStagePath(const FString& InPath) const;
#if WITH_EDITORONLY_DATA
	FString GetUnrealGeneratedSoundBanksPath(const FName& InPath) const { return GetUnrealGeneratedSoundBanksPath(InPath.ToString());}
	FString GetUnrealGeneratedSoundBanksPath(const FString& InPath) const;
#endif

	virtual EWwiseResourceLoaderState GetResourceLoaderState();
	virtual void SetResourceLoaderState(EWwiseResourceLoaderState State);
	virtual bool IsEnabled();
	virtual void Disable();
	virtual void Enable();

	virtual void SetLanguage(const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage);
	void SetPlatform(const FWwiseSharedPlatformId& InPlatform);

	virtual FWwiseLoadedAuxBus CreateAuxBusNode(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadAuxBusAsync(FWwiseLoadedAuxBusPromise&& Promise, FWwiseLoadedAuxBus&& InAuxBusListNode);
	virtual void UnloadAuxBusAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBus&& InAuxBusListNode);

	virtual FWwiseLoadedEvent CreateEventNode(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadEventAsync(FWwiseLoadedEventPromise&& Promise, FWwiseLoadedEvent&& InEventListNode);
	virtual void UnloadEventAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEvent&& InEventListNode);

	virtual FWwiseLoadedExternalSource CreateExternalSourceNode(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	virtual void LoadExternalSourceAsync(FWwiseLoadedExternalSourcePromise&& Promise, FWwiseLoadedExternalSource&& InExternalSourceListNode);
	virtual void UnloadExternalSourceAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSource&& InExternalSourceListNode);

	virtual FWwiseLoadedGroupValue CreateGroupValueNode(const FWwiseGroupValueCookedData& InGroupValueCookedData);
	virtual void LoadGroupValueAsync(FWwiseLoadedGroupValuePromise&& Promise, FWwiseLoadedGroupValue&& InGroupValueListNode);
	virtual void UnloadGroupValueAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValue&& InGroupValueListNode);

	virtual FWwiseLoadedInitBank CreateInitBankNode(const FWwiseInitBankCookedData& InInitBankCookedData);
	virtual void LoadInitBankAsync(FWwiseLoadedInitBankPromise&& Promise, FWwiseLoadedInitBank&& InInitBankListNode);
	virtual void UnloadInitBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBank&& InInitBankListNode);

	virtual FWwiseLoadedMedia CreateMediaNode(const FWwiseMediaCookedData& InMediaCookedData);
	virtual void LoadMediaAsync(FWwiseLoadedMediaPromise&& Promise, FWwiseLoadedMedia&& InMediaListNode);
	virtual void UnloadMediaAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMedia&& InMediaListNode);

	virtual FWwiseLoadedShareSet CreateShareSetNode(const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadShareSetAsync(FWwiseLoadedShareSetPromise&& Promise, FWwiseLoadedShareSet&& InShareSetListNode);
	virtual void UnloadShareSetAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSet&& InShareSetListNode);

	virtual FWwiseLoadedSoundBank CreateSoundBankNode(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadSoundBankAsync(FWwiseLoadedSoundBankPromise&& Promise, FWwiseLoadedSoundBank&& InSoundBankListNode);
	virtual void UnloadSoundBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBank&& InSoundBankListNode);

protected:
	using FLoadFileCallback = TUniqueFunction<void(bool bInResult)>;
	using FUnloadFileCallback = TUniqueFunction<void()>;
	using FCompletionPromise = TPromise<void>;
	using FCompletionFuture = TFuture<void>;
	using FCompletionFutureArray = TArray<FCompletionFuture>;

	FWwiseLoadedAuxBusList LoadedAuxBusList;
	FWwiseLoadedSoundBankList LoadedSoundBankList;
	FWwiseLoadedEventList LoadedEventList;
	FWwiseLoadedExternalSourceList LoadedExternalSourceList;
	FWwiseLoadedGroupValueList LoadedGroupValueList;
	FWwiseLoadedInitBankList LoadedInitBankList;
	FWwiseLoadedMediaList LoadedMediaList;
	FWwiseLoadedShareSetList LoadedShareSetList;

	TSet<FWwiseSwitchContainerLoadedGroupValueInfo> LoadedGroupValueInfo;

	FWwiseExecutionQueue ListExecutionQueue;
	FWwiseExecutionQueue SwitchContainerExecutionQueue;
	mutable FWwiseExecutionQueue FileExecutionQueue;

	virtual bool LoadAuxBusResources(FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData);
	virtual bool LoadEventResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void LoadEventSwitchContainerResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual bool LoadExternalSourceResources(FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData);
	virtual bool LoadGroupValueResources(FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData);
	virtual bool LoadInitBankResources(FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData);
	virtual bool LoadMediaResources(FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData);
	virtual bool LoadShareSetResources(FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData);
	virtual bool LoadSoundBankResources(FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData);
	virtual void LoadSwitchContainerLeafResources(FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData& LoadedData, const FWwiseSwitchContainerLeafCookedData& InCookedData);

	virtual void UnloadAuxBusResources(FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData);
	virtual void UnloadEventResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void UnloadEventSwitchContainerResources(FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void UnloadExternalSourceResources(FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData);
	virtual void UnloadGroupValueResources(FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData);
	virtual void UnloadInitBankResources(FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData);
	virtual void UnloadMediaResources(FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData);
	virtual void UnloadShareSetResources(FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData);
	virtual void UnloadSoundBankResources(FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData);
	virtual void UnloadSwitchContainerLeafResources(FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData& LoadedData, const FWwiseSwitchContainerLeafCookedData& InCookedData);

	virtual void AttachAuxBusNode(FWwiseLoadedAuxBusListNode* AuxBusListNode);
	virtual void AttachEventNode(FWwiseLoadedEventListNode* EventListNode);
	virtual void AttachExternalSourceNode(FWwiseLoadedExternalSourceListNode* ExternalSourceListNode);
	virtual void AttachGroupValueNode(FWwiseLoadedGroupValueListNode* GroupValueListNode);
	virtual void AttachInitBankNode(FWwiseLoadedInitBankListNode* InitBankListNode);
	virtual void AttachMediaNode(FWwiseLoadedMediaListNode* MediaListNode);
	virtual void AttachShareSetNode(FWwiseLoadedShareSetListNode* ShareSetListNode);
	virtual void AttachSoundBankNode(FWwiseLoadedSoundBankListNode* SoundBankListNode);

	virtual void DetachAuxBusNode(FWwiseLoadedAuxBusListNode* AuxBusListNode);
	virtual void DetachEventNode(FWwiseLoadedEventListNode* EventListNode);
	virtual void DetachExternalSourceNode(FWwiseLoadedExternalSourceListNode* ExternalSourceListNode);
	virtual void DetachGroupValueNode(FWwiseLoadedGroupValueListNode* GroupValueListNode);
	virtual void DetachInitBankNode(FWwiseLoadedInitBankListNode* InitBankListNode);
	virtual void DetachMediaNode(FWwiseLoadedMediaListNode* MediaListNode);
	virtual void DetachShareSetNode(FWwiseLoadedShareSetListNode* ShareSetListNode);
	virtual void DetachSoundBankNode(FWwiseLoadedSoundBankListNode* SoundBankListNode);

	void AddLoadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
	                                  const TArray<FWwiseExternalSourceCookedData>& InExternalSources, const TCHAR* InType, FName InDebugName, uint32 InShortId) const;
	void AddUnloadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources) const;
	void AddLoadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia,
	                         const TArray<FWwiseMediaCookedData>& InMedia, const TCHAR* InType, FName InDebugName, uint32 InShortId) const;
	void AddUnloadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia) const;
	void AddLoadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
	                             const TArray<FWwiseSoundBankCookedData>& InSoundBank, const TCHAR* InType, FName InDebugName, uint32 InShortId) const;
	void AddUnloadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks) const;
	void WaitForFutures(FCompletionFutureArray& FutureArray) const;

	void LoadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FLoadFileCallback&& InCallback) const;
	void UnloadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FUnloadFileCallback&& InCallback) const;
	void LoadMediaFile(const FWwiseMediaCookedData& InMedia, FLoadFileCallback&& InCallback) const;
	void UnloadMediaFile(const FWwiseMediaCookedData& InMedia, FUnloadFileCallback&& InCallback) const;
	void LoadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FLoadFileCallback&& InCallback) const;
	void UnloadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FUnloadFileCallback&& InCallback) const;

	template<typename MapValue>
	inline const FWwiseLanguageCookedData* GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FName& InDebugName) const;
};

template<typename MapValue>
inline const FWwiseLanguageCookedData* FWwiseResourceLoaderImpl::GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FName& InDebugName) const
{
	if (InLanguageOverride)
	{
		if (Map.Find(*InLanguageOverride))
		{
			return InLanguageOverride;
		}
		UE_LOG(LogWwiseResourceLoader, Log, TEXT("GetLanguageMapKey: Could not find overridden language %s while processing asset %s. Defaulting to language %s"),
			*InLanguageOverride->GetLanguageName().ToString(), *InDebugName.ToString(), *CurrentLanguage.GetLanguageName().ToString());
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
		*CurrentLanguage.GetLanguageName().ToString(), *InDebugName.ToString());
	return nullptr;
}

