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


#include "AkAudioModule.h"
#include "AkAudioDevice.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkUnrealHelper.h"
#include "AkWaapiClient.h"
#include "SReloadPopup.h"

#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#include "Misc/CoreDelegates.h"
#include "Misc/ScopedSlowTask.h"

#include "Wwise/LowLevel/WwiseLowLevelSoundEngine.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseDataStructure.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"
#endif

#if WITH_EDITOR
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#endif
#include <Runtime/Core/Public/Async/Async.h>

IMPLEMENT_MODULE(FAkAudioModule, AkAudio)
#define LOCTEXT_NAMESPACE "AkAudio"

FAkAudioModule* FAkAudioModule::AkAudioModuleInstance = nullptr;

// AkUnrealHelper overrides

namespace AkUnrealHelper
{
	static FString GetWwisePluginDirectoryImpl()
	{
		return FAkPlatform::GetWwisePluginDirectory();
	}

	static FString GetWwiseProjectPathImpl()
	{
		FString projectPath;

		if (auto* settings = GetDefault<UAkSettings>())
		{
			projectPath = settings->WwiseProjectPath.FilePath;

			if (FPaths::IsRelative(projectPath))
			{
				projectPath = FPaths::ConvertRelativePathToFull(GetProjectDirectory(), projectPath);
			}

#if PLATFORM_WINDOWS
			projectPath.ReplaceInline(TEXT("/"), TEXT("\\"));
#endif
		}

		return projectPath;
	}

	static FString GetSoundBankDirectoryImpl()
	{
		const UAkSettingsPerUser* UserSettings = GetDefault<UAkSettingsPerUser>();
		FString SoundBankDirectory;
		if (UserSettings && !UserSettings->GeneratedSoundBanksFolderUserOverride.Path.IsEmpty())
		{
			SoundBankDirectory = FPaths::Combine(GetContentDirectory(), UserSettings->GeneratedSoundBanksFolderUserOverride.Path);
		}
		else if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
		{
			SoundBankDirectory = FPaths::Combine(GetContentDirectory(), AkSettings->GeneratedSoundBanksFolder.Path);
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("AkUnrealHelper::GetSoundBankDirectory : Please set the Generated Soundbanks Folder in Wwise settings. Otherwise, sound will not function."));
			return {};
		}

		if(!SoundBankDirectory.EndsWith(TEXT("/")))
		{
			SoundBankDirectory.AppendChar('/');
		}

		return SoundBankDirectory;
	}
	static FString GetBaseAssetPackagePathImpl()
	{
		if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
		{
			return FPaths::Combine(TEXT("/Game/"), AkSettings->WwiseSoundDataFolder.Path);
		}

		return TEXT("/Game/WwiseAudio");
	}

	static FString GetStagePathImpl()
	{
		const UAkSettings* Settings = GetDefault<UAkSettings>();
#if WITH_EDITORONLY_DATA
		if (Settings && !Settings->WwiseStagingDirectory.Path.IsEmpty())
		{
			return Settings->WwiseStagingDirectory.Path;
		}
		return TEXT("WwiseAudio");
#endif
		if (Settings && !Settings->WwiseStagingDirectory.Path.IsEmpty())
		{
			return FPaths::ProjectContentDir() / Settings->WwiseStagingDirectory.Path;
		}
		return FPaths::ProjectContentDir() / TEXT("WwiseAudio");
	}
}

void FAkAudioModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAkAudioModule::OnPostEngineInit);

	IWwiseSoundEngineModule::ForceLoadModule();
	AkUnrealHelper::SetHelperFunctions(
		AkUnrealHelper::GetWwisePluginDirectoryImpl,
		AkUnrealHelper::GetWwiseProjectPathImpl,
		AkUnrealHelper::GetSoundBankDirectoryImpl,
		AkUnrealHelper::GetBaseAssetPackagePathImpl,
		AkUnrealHelper::GetStagePathImpl);
}

void FAkAudioModule::OnPostEngineInit()
{
#if WITH_EDITOR
	// It is not wanted to initialize the SoundEngine while running the GenerateSoundBanks commandlet.
	if (IsRunningCommandlet())
	{
		// We COULD use GetRunningCommandletClass(), but unfortunately it is set to nullptr in OnPostEngineInit.
		// We need to parse the command line.
		FString CmdLine(FCommandLine::Get());
		if (CmdLine.Contains(TEXT("-run=GenerateSoundBanks")))
		{
			return;
		}
	}
#endif

	if (AkAudioModuleInstance)
		return;

	AkAudioModuleInstance = this;

	FScopedSlowTask SlowTask(0, LOCTEXT("InitWwisePlugin", "Initializing Wwise Plug-in..."));

#if WITH_EDITOR
	// We expect the init bank to be in one of two locations
	// We won't be able to find it at this point if it has been moved
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FString> paths;
	IFileManager::Get().FindFilesRecursive(paths, *FPaths::ProjectContentDir(), TEXT("InitBank.uasset"), true, false);
	AssetRegistryModule.Get().ScanFilesSynchronous(paths, false);

	if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
	{
		AssetRegistryModule.Get().ScanPathsSynchronous({ AkSettings->DefaultAssetCreationPath }, false);
	}

#endif

	AkAudioDevice = new FAkAudioDevice;
	if (!AkAudioDevice)
		return;
	
	UpdateWwiseResourceLoaderSettings();

#if WITH_EDITORONLY_DATA
	UWwiseInitBankLoader::Get()->FindOrCreateInitBank();
	ParseGeneratedSoundBankData();
#else
	UWwiseInitBankLoader::Get()->FindInitBank();
#endif

	if (!AkAudioDevice->Init())
	{
		delete AkAudioDevice;
		AkAudioDevice = nullptr;
		return;
	}

	UWwiseInitBankLoader::Get()->LoadInitBank();

	OnTick = FTickerDelegate::CreateRaw(AkAudioDevice, &FAkAudioDevice::Update);
	TickDelegateHandle = FCoreTickerType::GetCoreTicker().AddTicker(OnTick);

#if WITH_EDITOR
	//We want to reload asset data after having parsed genererated SoundBank data AND after initializing the AkAudioDevice
	ReloadWwiseAssetData();
	FAkAudioStyle::Initialize();

	if (UAkSettings* Settings = GetMutableDefault<UAkSettings>())
	{
		Settings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &FAkAudioModule::OnSoundBanksFolderChanged);
	}
	if (UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>())
	{
		UserSettings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &FAkAudioModule::OnSoundBanksFolderChanged);
	}
	OnDatabaseUpdateCompleteHandle = FWwiseProjectDatabaseDelegates::Get().GetOnDatabaseUpdateCompletedDelegate().AddRaw(this, &FAkAudioModule::AssetReloadPrompt);

#if AK_SUPPORT_WAAPI
	if (!IsRunningCommandlet())
	{
		FAkWaapiClient::Initialize();
		if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
		{
			AkSettings->InitWaapiSync();
		}
	}
#endif
#endif
}

#if WITH_EDITOR
void FAkAudioModule::OnSoundBanksFolderChanged()
{
	UpdateWwiseResourceLoaderSettings();
	ParseGeneratedSoundBankData();
}
#endif

void FAkAudioModule::ShutdownModule()
{
	FCoreTickerType::GetCoreTicker().RemoveTicker(TickDelegateHandle);

	if (AkAudioDevice)
	{
		AkAudioDevice->Teardown();
		delete AkAudioDevice;
		AkAudioDevice = nullptr;
	}

#if WITH_EDITOR
	FAkAudioStyle::Shutdown();
#if AK_SUPPORT_WAAPI
	FAkWaapiClient::DeleteInstance();
#endif
#endif

	if (IWwiseSoundEngineModule::IsAvailable())
	{
		AkUnrealHelper::SetHelperFunctions(nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	AkAudioModuleInstance = nullptr;
}

FAkAudioDevice* FAkAudioModule::GetAkAudioDevice()
{
	return AkAudioDevice;
}

#if WITH_EDITOR
void FAkAudioModule::AssetReloadPrompt()
{
	const UAkSettingsPerUser* UserSettings = GetDefault<UAkSettingsPerUser>();
	if (UserSettings->AskForWwiseAssetsReload)
	{
		OpenAssetReloadPopup();
	}
	else
	{
		ReloadWwiseAssetData();
	}
}

void FAkAudioModule::OpenAssetReloadPopup()
{
	ReloadPopup.HideRefreshNotification();
	ReloadPopup.NotifyProjectRefresh();
}

void FAkAudioModule::ReloadWwiseAssetDataAsync()
{
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		ReloadWwiseAssetData();
		ReloadPopup.HideRefreshNotification();
	});
}

void FAkAudioModule::ReloadWwiseAssetData()
{
	if (FAkAudioDevice::IsInitialized())
	{
		UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::ReloadWwiseAssetData : Reloading Wwise asset data."));
		if (AkAudioDevice)
		{
			AkAudioDevice->StopAllSounds();
			auto* SoundEngine = FWwiseLowLevelSoundEngine::Get();
			if (LIKELY(SoundEngine))
			{
				SoundEngine->RenderAudio();
				FPlatformProcess::Sleep(0.1f);
			}
		}

		for (TObjectIterator<UAkAudioType> AudioAssetIt; AudioAssetIt; ++AudioAssetIt)
		{
			AudioAssetIt->UnloadData();
		}

		UWwiseInitBankLoader::Get()->ReloadInitBank();

		for (TObjectIterator<UAkAudioType> AudioAssetIt; AudioAssetIt; ++AudioAssetIt)
		{
			AudioAssetIt->LoadData();
		}
	}
	else
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("FAkAudioModule::ReloadWwiseAssetData : Skipping asset data reload because the SoundEngine is not initialized."));
	}
}
#endif

void FAkAudioModule::UpdateWwiseResourceLoaderSettings() const
{
	UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::UpdateWwiseResourceLoaderSettings : Updating Resource Loader settings."));

	FWwiseResourceLoaderImplWriteScopeLock ResourceLoader(UWwiseResourceLoader::Get());

	ResourceLoader->StagePath = AkUnrealHelper::GetStagePathImpl();

#if WITH_EDITORONLY_DATA
	ResourceLoader->GeneratedSoundBanksPath = FDirectoryPath{AkUnrealHelper::GetSoundBankDirectory()};
#endif
}

#if WITH_EDITORONLY_DATA
void FAkAudioModule::ParseGeneratedSoundBankData() const
{
	UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::ParseGeneratedSoundBankData : Parsing Wwise project data."));

	auto* ProjectDatabase = UWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioModule::ParseGeneratedSoundBankData : Could not get UWwiseProjectDatabase instance. Generated sound data will not be parsed."));
	}
	else
	{
		ProjectDatabase->UpdateDataStructure();
	}
}
#endif