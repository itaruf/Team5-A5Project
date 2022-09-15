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

#include "Wwise/WwiseGeneratedFiles.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/WwiseSharedLanguageId.h"

#include "Wwise/Info/WwiseAssetInfo.h"
#include "Wwise/Info/WwiseEventInfo.h"
#include "Wwise/Info/WwiseGroupValueInfo.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Ref/WwiseRefAcousticTexture.h"
#include "Wwise/Ref/WwiseAnyRef.h"
#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/Ref/WwiseRefBus.h"
#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/Ref/WwiseRefDialogueArgument.h"
#include "Wwise/Ref/WwiseRefExternalSource.h"
#include "Wwise/Ref/WwiseRefGameParameter.h"
#include "Wwise/Ref/WwiseRefLanguage.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Ref/WwiseRefPlatform.h"
#include "Wwise/Ref/WwiseRefPluginLib.h"
#include "Wwise/Ref/WwiseRefPluginShareset.h"
#include "Wwise/Ref/WwiseRefSoundBank.h"
#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Ref/WwiseRefSwitchContainer.h"
#include "Wwise/Ref/WwiseRefTrigger.h"

#include <inttypes.h>

struct WWISEPROJECTDATABASE_API FWwiseRootDataStructure
{
	FWwiseGeneratedFiles::FGeneratedRootFiles GeneratedRootFiles;
	WwiseMetadataFileMap JsonFiles;

	WwiseLanguageNamesMap LanguageNames;
	WwiseLanguageIdsMap LanguageIds;
	WwisePlatformNamesMap PlatformNames;
	WwisePlatformGuidsMap PlatformGuids;

	TSet<FWwiseSharedLanguageId> Languages;
	TSet<FWwiseSharedPlatformId> Platforms;

	FWwiseRootDataStructure() {}
	FWwiseRootDataStructure(WwiseMetadataFileMap&& JsonFiles);
	FWwiseRootDataStructure& operator+=(FWwiseRootDataStructure&& Rhs);

	uint32 GetLanguageId(const FString& Name) const
	{
		if (const auto* Language = LanguageNames.Find(Name))
		{
			return Language->LanguageId();
		}
		return FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE;
	}
};

struct WWISEPROJECTDATABASE_API FWwisePlatformDataStructure :
	private FWwiseRefAcousticTexture::FGlobalIdsMap,
	private FWwiseRefAudioDevice::FGlobalIdsMap,
	private FWwiseRefAuxBus::FGlobalIdsMap,
	private FWwiseRefBus::FGlobalIdsMap,
	private FWwiseRefCustomPlugin::FGlobalIdsMap,
	private FWwiseRefDialogueArgument::FGlobalIdsMap,
	private FWwiseRefDialogueEvent::FGlobalIdsMap,
	private FWwiseRefEvent::FGlobalIdsMap,
	private FWwiseRefExternalSource::FGlobalIdsMap,
	private FWwiseRefGameParameter::FGlobalIdsMap,
	private FWwiseRefMedia::FGlobalIdsMap,
	private FWwiseRefPluginLib::FGlobalIdsMap,
	private FWwiseRefPluginShareset::FGlobalIdsMap,
	private FWwiseRefSoundBank::FGlobalIdsMap,
	private FWwiseRefState::FGlobalIdsMap,
	private FWwiseRefStateGroup::FGlobalIdsMap,
	private FWwiseRefSwitch::FGlobalIdsMap,
	private FWwiseRefSwitchGroup::FGlobalIdsMap,
	private FWwiseRefTrigger::FGlobalIdsMap
{
	FWwiseSharedPlatformId Platform;
	FWwiseRefPlatform PlatformRef;

	FWwiseGeneratedFiles::FPlatformFiles GeneratedPlatformFiles;
	WwiseMetadataFileMap JsonFiles;

	WwiseAcousticTextureGlobalIdsMap& AcousticTextures;
	WwiseAudioDeviceGlobalIdsMap& AudioDevices;
	WwiseAuxBusGlobalIdsMap& AuxBusses;
	WwiseBusGlobalIdsMap& Busses;
	WwiseCustomPluginGlobalIdsMap& CustomPlugins;
	WwiseDialogueArgumentGlobalIdsMap& DialogueArguments;
	WwiseDialogueEventGlobalIdsMap& DialogueEvents;
	WwiseEventGlobalIdsMap& Events;
	WwiseExternalSourceGlobalIdsMap& ExternalSources;
	WwiseGameParameterGlobalIdsMap& GameParameters;
	WwiseMediaGlobalIdsMap& MediaFiles;
	WwisePluginLibGlobalIdsMap& PluginLibs;
	WwisePluginSharesetGlobalIdsMap& PluginSharesets;
	WwiseSoundBankGlobalIdsMap& SoundBanks;
	WwiseStateGlobalIdsMap& States;
	WwiseStateGroupGlobalIdsMap& StateGroups;
	WwiseSwitchGlobalIdsMap& Switches;
	WwiseSwitchGroupGlobalIdsMap& SwitchGroups;
	WwiseTriggerGlobalIdsMap& Triggers;

	WwisePluginLibNamesMap PluginLibNames;
	WwiseSwitchContainersByEvent SwitchContainersByEvent;
	WwiseGuidMap Guids;
	WwiseNameMap Names;

	FWwisePlatformDataStructure();
	FWwisePlatformDataStructure(const FWwiseSharedPlatformId& InPlatform, FWwiseRootDataStructure& InRootData, WwiseMetadataFileMap&& InJsonFiles);
	FWwisePlatformDataStructure(const FWwisePlatformDataStructure& Rhs);
	FWwisePlatformDataStructure(FWwisePlatformDataStructure&& Rhs);
	FWwisePlatformDataStructure& operator+=(FWwisePlatformDataStructure&& Rhs);

	template <typename RequiredRef>
	void GetRefMap(TMap<FWwiseSharedLanguageId, RequiredRef>& OutRefMap, const TSet<FWwiseSharedLanguageId>& InLanguages, const FWwiseAssetInfo& InInfo) const;

	void GetRefMap(TMap<FWwiseSharedLanguageId, TSet<FWwiseRefEvent>>& OutRefMap, const TSet<FWwiseSharedLanguageId>& InLanguages, const FWwiseEventInfo& InInfo) const;

	template <typename RequiredRef>
	void GetRefMap(TMap<FWwiseSharedLanguageId, RequiredRef>& OutRefMap, const TSet<FWwiseSharedLanguageId>& InLanguages, const FWwiseGroupValueInfo& InInfo) const;

	template <typename RequiredRef>
	bool GetRef(RequiredRef& OutRef, const FWwiseSharedLanguageId& InLanguage, const FWwiseAssetInfo& InInfo) const;

	bool GetRef(TSet<FWwiseRefEvent>& OutRef, const FWwiseSharedLanguageId& InLanguage, const FWwiseEventInfo& InInfo) const;

	template <typename RequiredRef>
	bool GetRef(RequiredRef& OutRef, const FWwiseSharedLanguageId& InLanguage, const FWwiseGroupValueInfo& InInfo) const;

	template <typename RequiredRef>
	static bool GetLocalizableRef(RequiredRef& OutRef, const TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& InGlobalMap,
		uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName);
	
	template <typename RequiredRef>
	static bool GetLocalizableGroupRef(RequiredRef& OutRef, const TMap<FWwiseDatabaseLocalizableGroupValueKey, RequiredRef>& InGlobalMap,
		FWwiseDatabaseGroupValueKey InGroupValue, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName);

	static void GetEventRefs(TArray<FWwiseRefEvent>& OutRefs, const TMap<FWwiseDatabaseEventIdKey, FWwiseRefEvent>& InGlobalMap,
		uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName);

	template <typename RefType>
	bool GetFromId(RefType& OutRef, uint32 InId, uint32 InLanguageId = 0, uint32 InSoundBankId = 0) const
	{
		return GetLocalizableRef(OutRef, RefType::FGlobalIdsMap::GlobalIdsMap, InId, InLanguageId, InSoundBankId, RefType::NAME);
	}

	template <typename RefType>
	bool GetFromId(RefType& OutRef, FWwiseDatabaseGroupValueKey InId, uint32 InLanguageId = 0, uint32 InSoundBankId = 0) const
	{
		return GetLocalizableGroupRef(OutRef, RefType::FGlobalIdsMap::GlobalIdsMap, InId, InLanguageId, InSoundBankId, RefType::NAME);
	}

	bool GetFromId(FWwiseRefMedia& OutRef, uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId) const;
	bool GetFromId(TSet<FWwiseRefEvent>& OutRef, uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId) const;

	template <typename RequiredRef>
	void AddBasicRefToMap(TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const FWwiseMetadataBasicReference& InObject);

	template <typename RequiredRef>
	void AddEventRefToMap(TMap<FWwiseDatabaseEventIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const FWwiseMetadataBasicReference& InObject);

	template <typename RequiredRef>
	void AddRefToMap(TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const uint32& InId, const FString* InName, const FString* InObjectPath, const FGuid* InGuid);

	template <typename RequiredRef>
	void AddRefToMap(TMap<FWwiseDatabaseEventIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const uint32& InId, const FString* InName, const FString* InObjectPath, const FGuid* InGuid);

private:
	FWwisePlatformDataStructure& operator=(const FWwisePlatformDataStructure& Rhs) = delete;
};

struct WWISEPROJECTDATABASE_API FWwiseDataStructure
{
	FRWLock Lock;

	FWwiseRootDataStructure RootData;
	TMap<FWwiseSharedPlatformId, FWwisePlatformDataStructure> Platforms;

	FWwiseDataStructure() {}
	FWwiseDataStructure(const FDirectoryPath& InDirectoryPath, const FString* InPlatform = nullptr, const FGuid* InBasePlatformGuid = nullptr);
	~FWwiseDataStructure();

	FWwiseDataStructure& operator+=(FWwiseDataStructure&& Rhs);
	FWwiseDataStructure& operator=(FWwiseDataStructure&& Rhs)
	{
		RootData = MoveTemp(Rhs.RootData);
		Platforms = MoveTemp(Rhs.Platforms);
		return *this;
	}

private:
	FWwiseDataStructure(const FWwiseDataStructure& other) = delete;
	FWwiseDataStructure& operator=(const FWwiseDataStructure& other) = delete;
	void LoadDataStructure(FWwiseGeneratedFiles&& Directory);
};


template<typename RequiredRef>
inline void FWwisePlatformDataStructure::GetRefMap(TMap<FWwiseSharedLanguageId, RequiredRef>& OutRefMap, const TSet<FWwiseSharedLanguageId>& InLanguages, const FWwiseAssetInfo& InInfo) const
{
	OutRefMap.Empty(InLanguages.Num());
	for (const auto& Language : InLanguages)
	{
		RequiredRef Ref;
		if (GetRef(Ref, Language, InInfo))
		{
			OutRefMap.Add(Language, Ref);
		}
	}
}


template<typename RequiredRef>
inline void FWwisePlatformDataStructure::GetRefMap(TMap<FWwiseSharedLanguageId, RequiredRef>& OutRefMap, const TSet<FWwiseSharedLanguageId>& InLanguages, const FWwiseGroupValueInfo& InInfo) const
{
	OutRefMap.Empty(InLanguages.Num());
	for (const auto& Language : InLanguages)
	{
		RequiredRef Ref;
		if (GetRef(Ref, Language, InInfo))
		{
			OutRefMap.Add(Language, Ref);
		}
	}
}

template <typename RequiredRef>
inline bool FWwisePlatformDataStructure::GetRef(RequiredRef& OutRef, const FWwiseSharedLanguageId& InLanguage, const FWwiseAssetInfo& InInfo) const
{
	const auto LanguageId = InLanguage.GetLanguageId();

	// Get from GUID
	{
		FWwiseDatabaseLocalizableGuidKey LocalizableGuid(InInfo.AssetGuid, LanguageId);
		const auto* AssetFromGuid = Guids.Find(LocalizableGuid);
		if (LIKELY(AssetFromGuid))
		{
			return AssetFromGuid->GetRef(OutRef);
		}
	}
	if (LIKELY(LanguageId != 0))
	{
		FWwiseDatabaseLocalizableGuidKey LocalizableGuid(InInfo.AssetGuid, 0);
		const auto* AssetFromGuid = Guids.Find(LocalizableGuid);
		if (LIKELY(AssetFromGuid))
		{
			return AssetFromGuid->GetRef(OutRef);
		}
	}

	// Get from Short ID
	if (GetFromId(OutRef, InInfo.AssetShortId, LanguageId, InInfo.HardCodedSoundBankShortId))
	{
		return true;
	}

	// Get from Name. Try all found assets with such name until we get one
	TArray<const FWwiseAnyRef*> FoundAssets;
	Names.MultiFindPointer(FWwiseDatabaseLocalizableNameKey(InInfo.AssetName, LanguageId), FoundAssets);
	if (LIKELY(LanguageId != 0))
	{
		Names.MultiFindPointer(FWwiseDatabaseLocalizableNameKey(InInfo.AssetName, 0), FoundAssets);
	}
	for (const auto* Asset : FoundAssets)
	{
		if (Asset->GetRef(OutRef))
		{
			return true;
		}
	}
	return false;
}

template <typename RequiredRef>
inline bool FWwisePlatformDataStructure::GetRef(RequiredRef& OutRef, const FWwiseSharedLanguageId& InLanguage, const FWwiseGroupValueInfo& InInfo) const
{
	const auto LanguageId = InLanguage.GetLanguageId();

	// Get from GUID
	{
		FWwiseDatabaseLocalizableGuidKey LocalizableGuid(InInfo.AssetGuid, LanguageId);
		const auto* AssetFromGuid = Guids.Find(LocalizableGuid);
		if (LIKELY(AssetFromGuid))
		{
			return AssetFromGuid->GetRef(OutRef);
		}
	}
	if (LIKELY(LanguageId != 0))
	{
		FWwiseDatabaseLocalizableGuidKey LocalizableGuid(InInfo.AssetGuid, 0);
		const auto* AssetFromGuid = Guids.Find(LocalizableGuid);
		if (LIKELY(AssetFromGuid))
		{
			return AssetFromGuid->GetRef(OutRef);
		}
	}

	// Get from Short ID
	if (GetFromId(OutRef, FWwiseDatabaseGroupValueKey(InInfo.GroupShortId, InInfo.AssetShortId), InLanguage.GetLanguageId(), 0))
	{
		return true;
	}

	// Get from Name. Try all found assets with such name until we get one
	FWwiseDatabaseLocalizableNameKey LocalizableName(InInfo.AssetName, InLanguage.GetLanguageId());
	TArray<const FWwiseAnyRef*> FoundAssets;
	Names.MultiFindPointer(FWwiseDatabaseLocalizableNameKey(InInfo.AssetName, LanguageId), FoundAssets);
	if (LIKELY(LanguageId != 0))
	{
		Names.MultiFindPointer(FWwiseDatabaseLocalizableNameKey(InInfo.AssetName, 0), FoundAssets);
	}
	for (const auto* Asset : FoundAssets)
	{
		if (Asset->GetRef(OutRef))
		{
			return true;
		}
	}
	return false;
}

template <typename RequiredRef>
inline bool FWwisePlatformDataStructure::GetLocalizableRef(RequiredRef& OutRef, const TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& InGlobalMap,
	uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName)
{
	const RequiredRef* Result = nullptr;
	if (LIKELY(InLanguageId != 0))
	{
		FWwiseDatabaseLocalizableIdKey LocalizableId(InShortId, InLanguageId);
		Result = InGlobalMap.Find(LocalizableId);

		if (!Result)
		{
			FWwiseDatabaseLocalizableIdKey NoLanguageId(InShortId, FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE);
			Result = InGlobalMap.Find(NoLanguageId);
		}
	}
	else
	{
		for (const auto& Elem : InGlobalMap)
		{
			if (Elem.Key.Id == InShortId)
			{
				Result = &Elem.Value;
				break;
			}
		}
	}

	if (UNLIKELY(!Result))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Could not find %s %" PRIu32 " (Lang=%" PRIu32 "; SB=%" PRIu32 ")"), InDebugName, InShortId, InLanguageId, InSoundBankId);
		return false;
	}

	if (InSoundBankId != 0)
	{
		const FWwiseMetadataSoundBank* SoundBank = Result->GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not retrieve SoundBank for %s %" PRIu32 " (Lang=%" PRIu32 "; SB=%" PRIu32 ")"), InDebugName, InShortId, InLanguageId, InSoundBankId);
			return false;
		}
		if (UNLIKELY(SoundBank->Id != InSoundBankId))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Get incorrect SoundBank %" PRIu32 " for %s %" PRIu32 " (Lang = %" PRIu32 "; SB = %" PRIu32 ")"), SoundBank->Id, InDebugName, InShortId, InLanguageId, InSoundBankId);
			return false;
		}
	}

	OutRef = *Result;
	return true;
}

template <>
inline bool FWwisePlatformDataStructure::GetLocalizableRef<FWwiseRefPluginLib>(FWwiseRefPluginLib& OutRef, const TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefPluginLib>& InGlobalMap,
	uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName)
{
	const FWwiseRefPluginLib* Result = nullptr;
	if (LIKELY(InLanguageId != 0))
	{
		FWwiseDatabaseLocalizableIdKey LocalizableId(InShortId, InLanguageId);
		Result = InGlobalMap.Find(LocalizableId);

		if (!Result)
		{
			FWwiseDatabaseLocalizableIdKey NoLanguageId(InShortId, FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE);
			Result = InGlobalMap.Find(NoLanguageId);
		}
	}
	else
	{
		for (const auto& Elem : InGlobalMap)
		{
			if (Elem.Key.Id == InShortId)
			{
				Result = &Elem.Value;
				break;
			}
		}
	}

	if (UNLIKELY(!Result))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Could not find %s %" PRIu32 " (Lang=%" PRIu32 "; SB=%" PRIu32 ")"), InDebugName, InShortId, InLanguageId, InSoundBankId);
		return false;
	}

	OutRef = *Result;
	return true;
}

template <typename RequiredRef>
inline bool FWwisePlatformDataStructure::GetLocalizableGroupRef(RequiredRef& OutRef, const TMap<FWwiseDatabaseLocalizableGroupValueKey, RequiredRef>& InGlobalMap,
	FWwiseDatabaseGroupValueKey InGroupValue, uint32 InLanguageId, uint32 InSoundBankId, const TCHAR* InDebugName)
{
	const RequiredRef* Result = nullptr;
	if (LIKELY(InLanguageId != 0))
	{
		FWwiseDatabaseLocalizableGroupValueKey LocalizableGroupValue(InGroupValue, InLanguageId);
		Result = InGlobalMap.Find(LocalizableGroupValue);

		if (!Result)
		{
			FWwiseDatabaseLocalizableGroupValueKey NoLanguageId(InGroupValue, FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE);
			Result = InGlobalMap.Find(NoLanguageId);
		}
	}
	else
	{
		for (const auto& Elem : InGlobalMap)
		{
			if (Elem.Key.GroupValue == InGroupValue)
			{
				Result = &Elem.Value;
				break;
			}
		}
	}

	if (UNLIKELY(!Result))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Could not find %s %" PRIu32 " %" PRIu32 " (Lang=%" PRIu32 "; SB=%" PRIu32 ")"), InDebugName, InGroupValue.GroupId, InGroupValue.Id, InLanguageId, InSoundBankId);
		return false;
	}

	if (InSoundBankId != 0)
	{
		const FWwiseMetadataSoundBank* SoundBank = Result->GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not retrieve SoundBank for %s %" PRIu32 " %" PRIu32 " (Lang = %" PRIu32 "; SB = %" PRIu32 ")"), InDebugName, InGroupValue.GroupId, InGroupValue.Id, InLanguageId, InSoundBankId);
			return false;
		}
		if (UNLIKELY(SoundBank->Id != InSoundBankId))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Get incorrect SoundBank %" PRIu32 " for %s %" PRIu32 " %" PRIu32 " (Lang = %" PRIu32 "; SB = %" PRIu32 ")"), SoundBank->Id, InDebugName, InGroupValue.GroupId, InGroupValue.Id, InLanguageId, InSoundBankId);
			return false;
		}
	}

	OutRef = *Result;
	return true;
}

template<typename RequiredRef>
inline void FWwisePlatformDataStructure::AddBasicRefToMap(TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const FWwiseMetadataBasicReference& InObject)
{
	AddRefToMap(OutMap, InRef, InObject.Id, &InObject.Name, &InObject.ObjectPath, &InObject.GUID);
}

template<typename RequiredRef>
inline void FWwisePlatformDataStructure::AddEventRefToMap(TMap<FWwiseDatabaseEventIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const FWwiseMetadataBasicReference& InObject)
{
	AddRefToMap(OutMap, InRef, InObject.Id, &InObject.Name, &InObject.ObjectPath, &InObject.GUID);
}

template<typename RequiredRef>
void FWwisePlatformDataStructure::AddRefToMap(TMap<FWwiseDatabaseLocalizableIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const uint32& InId, const FString* InName, const FString* InObjectPath, const FGuid* InGuid)
{
	const auto AnyRef = FWwiseAnyRef::Create(InRef);
	if (InName && !InName->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InName, InRef.LanguageId), AnyRef);
	}
	if (InObjectPath && !InObjectPath->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InObjectPath, InRef.LanguageId), AnyRef);
	}
	if (InGuid && InGuid->IsValid())
	{
		Guids.Add(FWwiseDatabaseLocalizableGuidKey(*InGuid, InRef.LanguageId), AnyRef);
	}
	OutMap.Add(FWwiseDatabaseLocalizableIdKey(InId, InRef.LanguageId), InRef);
}

template<typename RequiredRef>
void FWwisePlatformDataStructure::AddRefToMap(TMap<FWwiseDatabaseEventIdKey, RequiredRef>& OutMap, const RequiredRef& InRef, const uint32& InId, const FString* InName, const FString* InObjectPath, const FGuid* InGuid)
{
	const auto AnyRef = FWwiseAnyRef::Create(InRef);
	if (InName && !InName->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InName, InRef.LanguageId), AnyRef);
	}
	if (InObjectPath && !InObjectPath->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InObjectPath, InRef.LanguageId), AnyRef);
	}
	if (InGuid && InGuid->IsValid())
	{
		Guids.Add(FWwiseDatabaseLocalizableGuidKey(*InGuid, InRef.LanguageId), AnyRef);
	}
	OutMap.Add(FWwiseDatabaseEventIdKey(InId, InRef.LanguageId, InRef.SoundBankId()), InRef);
}

template<>
inline void FWwisePlatformDataStructure::AddRefToMap<FWwiseRefPluginLib>(TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefPluginLib>& OutMap, const FWwiseRefPluginLib& InRef, const uint32& InId, const FString* InName, const FString* InObjectPath, const FGuid* InGuid)
{
	const auto AnyRef = FWwiseAnyRef::Create(InRef);
	if (InName && !InName->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InName, 0), AnyRef);
	}
	if (InObjectPath && !InObjectPath->IsEmpty())
	{
		Names.Add(FWwiseDatabaseLocalizableNameKey(*InObjectPath, 0), AnyRef);
	}
	if (InGuid && InGuid->IsValid())
	{
		Guids.Add(FWwiseDatabaseLocalizableGuidKey(*InGuid, 0), AnyRef);
	}
	OutMap.Add(FWwiseDatabaseLocalizableIdKey(InId, 0), InRef);
}
