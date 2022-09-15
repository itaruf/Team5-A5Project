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

#include "Wwise/WwiseDataStructure.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "EditorSubsystem.h"

#include "WwiseProjectDatabase.generated.h"

class UWwiseResourceLoader;
class UWwiseProjectDatabase;
using FSharedWwiseDataStructure = TSharedRef<FWwiseDataStructure>;

class WWISEPROJECTDATABASE_API FWwiseDataStructureScopeLock : public FRWScopeLock
{
public:
	FWwiseDataStructureScopeLock(const UWwiseProjectDatabase& InProjectDatabase);

	const FWwiseDataStructure& operator*() const
	{
		return DataStructure;
	}

	const FWwiseDataStructure* operator->() const
	{
		return &DataStructure;
	}

	const WwiseAcousticTextureGlobalIdsMap& GetAcousticTextures() const;
	FWwiseRefAcousticTexture GetAcousticTexture(const FWwiseAssetInfo& InInfo) const;

	const WwiseAudioDeviceGlobalIdsMap& GetAudioDevices() const;
	FWwiseRefAudioDevice GetAudioDevice(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseAuxBusGlobalIdsMap& GetAuxBusses() const;
	FWwiseRefAuxBus GetAuxBus(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseBusGlobalIdsMap& GetBusses() const;
	FWwiseRefBus GetBus(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseCustomPluginGlobalIdsMap& GetCustomPlugins() const;
	FWwiseRefCustomPlugin GetCustomPlugin(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseDialogueArgumentGlobalIdsMap& GetDialogueArguments() const;
	FWwiseRefDialogueArgument GetDialogueArgument(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseDialogueEventGlobalIdsMap& GetDialogueEvents() const;
	FWwiseRefDialogueEvent GetDialogueEvent(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseEventGlobalIdsMap& GetEvents() const;
	TSet<FWwiseRefEvent> GetEvent(const FWwiseEventInfo& InInfo) const;
	
	const WwiseExternalSourceGlobalIdsMap& GetExternalSources() const;
	FWwiseRefExternalSource GetExternalSource(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseGameParameterGlobalIdsMap& GetGameParameters() const;
	FWwiseRefGameParameter GetGameParameter(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseMediaGlobalIdsMap& GetMediaFiles() const;
	FWwiseRefMedia GetMediaFile(const FWwiseAssetInfo& InInfo) const;
	
	const WwisePluginLibGlobalIdsMap& GetPluginLibs() const;
	FWwiseRefPluginLib GetPluginLib(const FWwiseAssetInfo& InInfo) const;
	
	const WwisePluginSharesetGlobalIdsMap& GetPluginSharesets() const;
	FWwiseRefPluginShareset GetPluginShareset(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseSoundBankGlobalIdsMap& GetSoundBanks() const;
	FWwiseRefSoundBank GetSoundBank(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseStateGlobalIdsMap& GetStates() const;
	FWwiseRefState GetState(const FWwiseGroupValueInfo& InInfo) const;
	
	const WwiseStateGroupGlobalIdsMap& GetStateGroups() const;
	FWwiseRefStateGroup GetStateGroup(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseSwitchGlobalIdsMap& GetSwitches() const;
	FWwiseRefSwitch GetSwitch(const FWwiseGroupValueInfo& InInfo) const;
	
	const WwiseSwitchGroupGlobalIdsMap& GetSwitchGroups() const;
	FWwiseRefSwitchGroup GetSwitchGroup(const FWwiseAssetInfo& InInfo) const;
	
	const WwiseTriggerGlobalIdsMap& GetTriggers() const;
	FWwiseRefTrigger GetTrigger(const FWwiseAssetInfo& InInfo) const;

	const TSet<FWwiseSharedLanguageId>& GetLanguages() const;
	const TSet<FWwiseSharedPlatformId>& GetPlatforms() const;
	FWwiseRefPlatform GetPlatform(const FWwiseSharedPlatformId& InPlatformId) const;

	const FWwisePlatformDataStructure* GetCurrentPlatformData() const;

	const FWwiseSharedLanguageId& GetCurrentLanguage() const { return CurrentLanguage; }
	const FWwiseSharedPlatformId& GetCurrentPlatform() const { return CurrentPlatform; }
	bool DisableDefaultPlatforms() const { return bDisableDefaultPlatforms; }
	
private:
	const FWwiseDataStructure& DataStructure;

	FWwiseSharedLanguageId CurrentLanguage;
	FWwiseSharedPlatformId CurrentPlatform;
	bool bDisableDefaultPlatforms;

	UE_NONCOPYABLE(FWwiseDataStructureScopeLock);
};

class WWISEPROJECTDATABASE_API FWwiseDataStructureWriteScopeLock : public FRWScopeLock
{
public:
	FWwiseDataStructureWriteScopeLock(UWwiseProjectDatabase& InProjectDatabase);

	FWwiseDataStructure& operator*()
	{
		return DataStructure;
	}

	FWwiseDataStructure* operator->()
	{
		return &DataStructure;
	}

private:
	FWwiseDataStructure& DataStructure;
	UE_NONCOPYABLE(FWwiseDataStructureWriteScopeLock);
};

UCLASS(Abstract)
class WWISEPROJECTDATABASE_API UWwiseProjectDatabase : public UEditorSubsystem
{
private:
	GENERATED_BODY()

	friend class FWwiseDataStructureScopeLock;
	friend class FWwiseDataStructureWriteScopeLock;

public:
	static const FGuid BasePlatformGuid;

	inline static UWwiseProjectDatabase* Get()
	{
		if (auto* Module = IWwiseProjectDatabaseModule::GetModule())
		{
			return Module->GetProjectDatabase();
		}
		return nullptr;
	}

	virtual void UpdateDataStructure(
		const FDirectoryPath* InUpdateGeneratedSoundBanksPath = nullptr,
		const FGuid* InBasePlatformGuid = &BasePlatformGuid) {}

	virtual void PrepareProjectDatabaseForPlatform(UWwiseResourceLoader* InResourceLoader) {}
	virtual UWwiseResourceLoader* GetResourceLoader() { return nullptr; }
	virtual const UWwiseResourceLoader* GetResourceLoader() const { return nullptr; }

	UFUNCTION()
	FWwiseSharedLanguageId GetCurrentLanguage() const;

	UFUNCTION()
	FWwiseSharedPlatformId GetCurrentPlatform() const;

protected:
	virtual FSharedWwiseDataStructure& GetLockedDataStructure() { check(false); UE_ASSUME(false); }
	virtual const FSharedWwiseDataStructure& GetLockedDataStructure() const { check(false); UE_ASSUME(false); }

	template <typename RequiredRef>
	bool GetRef(RequiredRef& OutRef, const FWwiseAssetInfo& InInfo)
	{
		const auto* ResourceLoader = GetResourceLoader();
		check(ResourceLoader);
		const auto& PlatformRef = ResourceLoader->GetCurrentPlatform();

		const auto& DataStructure = *GetLockedDataStructure();

		const auto* Platform = DataStructure.Platforms.Find(PlatformRef);
		if (UNLIKELY(!Platform))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("GetRef: Platform not found"));
			return false;
		}

		return Platform->GetRef(OutRef, InInfo);
	}

	bool DisableDefaultPlatforms() const;
};