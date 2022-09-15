/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "Engine/EngineTypes.h"
#include "AkWaapiClient.h"
#include "AkSettingsPerUser.generated.h"

DECLARE_EVENT(UAkSettingsPerUser, AutoConnectChanged);
DECLARE_EVENT(UAkSettingsPerUser, AutoSyncWaapiNamesChanged);
DECLARE_MULTICAST_DELEGATE(FOnSoundBanksPathChangedDelegate);

UCLASS(config = EditorPerProjectUserSettings)
class AKAUDIO_API UAkSettingsPerUser : public UObject
{
	GENERATED_BODY()

public:
	UAkSettingsPerUser(const FObjectInitializer& ObjectInitializer);

	// Wwise Installation Path (Root folder containing the Authoring, SDK, etc folders)
	UPROPERTY(Config, EditAnywhere, Category = "Installation")
	FDirectoryPath WwiseWindowsInstallationPath;

	// Wwise Installation Path (Root folder containing the Authoring, SDK, etc folders)
	UPROPERTY(Config, EditAnywhere, Category = "Installation", meta = (FilePathFilter = "app", AbsolutePath))
	FFilePath WwiseMacInstallationPath;

	//Override the Generated Soundbanks Path in the project settings
	UPROPERTY(Config, EditAnywhere, Category = "Installation")
	FDirectoryPath GeneratedSoundBanksFolderUserOverride;

	// IP Address used to connect to WAAPI. Changing this requires Editor restart
	UPROPERTY(Config, EditAnywhere, Category = "WAAPI")
	FString WaapiIPAddress = WAAPI_LOCAL_HOST_IP_STRING;

	// Network Port used to connect to WAAPI. Changing this requires Editor restart
	UPROPERTY(Config, EditAnywhere, Category = "WAAPI")
	uint32 WaapiPort = WAAPI_PORT;

	// Whether to connect to WAAPI or not
	UPROPERTY(Config, EditAnywhere, Category = "WAAPI")
	bool bAutoConnectToWAAPI = false;

	// Whether to synchronize the selection between the WAAPI picker and the Wwise Project Explorer
	UPROPERTY(Config, EditAnywhere, Category = "WAAPI")
	bool AutoSyncSelection = true;

	//Synchronize name changes made in the Wwise project over WAAPI (will only make changes when both projects are open)
	UPROPERTY(Config, EditAnywhere, Category = "WAAPI")
	bool bAutoSyncWwiseAssetNames = false;

	// Time out value for the xml error message translator to translate an error message (in ms). If set to 0, disable the translator entirely
	UPROPERTY(Config, EditAnywhere, Category = "Error Message Translator")
	uint32 XmlTranslatorTimeout = 10;

	// Time out value for the waapi error message translator to translate an error message (in ms). If set to 0, disable the translator entirely
	UPROPERTY(Config, EditAnywhere, Category = "Error Message Translator")
	uint32 WaapiTranslatorTimeout = 0;

	UPROPERTY(Config)
	bool SuppressWwiseProjectPathWarnings = false;

	UPROPERTY(Config)
	bool SoundDataGenerationSkipLanguage = false;

	//Will open a notification that must be accepted before reloading Wwise Asset Data
	UPROPERTY(Config, EditAnywhere, Category = "Assets Reload")
	bool AskForWwiseAssetsReload = false;


#if WITH_EDITOR

	public:
	mutable AutoConnectChanged OnAutoConnectToWaapiChanged;
	mutable AutoSyncWaapiNamesChanged OnAutoSyncWwiseAssetNamesChanged;

	FOnSoundBanksPathChangedDelegate OnGeneratedSoundBanksPathChanged;
protected:
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void PreEditChange(FProperty* PropertyAboutToChange) override;

private:
	FString PreviousWwiseWindowsInstallationPath;
	FString PreviousWwiseMacInstallationPath;
	FString PreviousGeneratedSoundBanksFolder;
#endif
};