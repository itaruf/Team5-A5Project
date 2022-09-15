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


/*------------------------------------------------------------------------------------
	AkAudioBankGenerationHelpers.cpp: Wwise Helpers to generate banks from the editor and when cooking.
------------------------------------------------------------------------------------*/

#include "AkAudioBankGenerationHelpers.h"

#include "AkAudioDevice.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkUnrealHelper.h"
#include "IAudiokineticTools.h"
#include "AssetManagement/AkAssetDatabase.h"

#include "Editor/UnrealEd/Public/ObjectTools.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Slate/Public/Framework/Application/SlateApplication.h"
#include "SlateCore/Public/Widgets/SWindow.h"
#include "UI/SClearSoundData.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "UI/SGenerateSoundBanks.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace AkAudioBankGenerationHelper
{
	FString GetWwiseConsoleApplicationPath()
	{
		const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		FString ApplicationToRun;
		ApplicationToRun.Empty();

		if (AkSettingsPerUser)
		{
#if PLATFORM_WINDOWS
			ApplicationToRun = AkSettingsPerUser->WwiseWindowsInstallationPath.Path;
#else
			ApplicationToRun = AkSettingsPerUser->WwiseMacInstallationPath.FilePath;
#endif
			if (FPaths::IsRelative(ApplicationToRun))
			{
				ApplicationToRun = FPaths::ConvertRelativePathToFull(AkUnrealHelper::GetProjectDirectory(), ApplicationToRun);
			}
			if (!(ApplicationToRun.EndsWith(TEXT("/")) || ApplicationToRun.EndsWith(TEXT("\\"))))
			{
				ApplicationToRun += TEXT("/");
			}

#if PLATFORM_WINDOWS
			if (FPaths::FileExists(ApplicationToRun + TEXT("Authoring/x64/Release/bin/WwiseConsole.exe")))
			{
				ApplicationToRun += TEXT("Authoring/x64/Release/bin/WwiseConsole.exe");
			}
			else
			{
				ApplicationToRun += TEXT("Authoring/Win32/Release/bin/WwiseConsole.exe");
			}
			ApplicationToRun.ReplaceInline(TEXT("/"), TEXT("\\"));
#elif PLATFORM_MAC
			ApplicationToRun += TEXT("Contents/Tools/WwiseConsole.sh");
			ApplicationToRun = TEXT("\"") + ApplicationToRun + TEXT("\"");
#endif
		}

		return ApplicationToRun;
	}

	void CreateGenerateSoundDataWindow(bool ProjectSave)
	{
		if (AkAssetDatabase::Get().CheckIfLoadingAssets())
		{
			return;
		}

		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("AkAudioGenerateSoundData", "Generate SoundBanks"))
			.ClientSize(FVector2D(600.f, 332.f))
			.SupportsMaximize(false).SupportsMinimize(false)
			.SizingRule(ESizingRule::FixedSize)
			.FocusWhenFirstShown(true);

		TSharedRef<SGenerateSoundBanks> WindowContent = SNew(SGenerateSoundBanks);
		if (!WindowContent->ShouldDisplayWindow())
		{
			return;
		}

		// Add our SGenerateSoundBanks to the window
		WidgetWindow->SetContent(WindowContent);

		// Set focus to our SGenerateSoundBanks widget, so our keyboard keys work right away
		WidgetWindow->SetWidgetToFocusOnActivate(WindowContent);

		// This creates a windows that blocks the rest of the UI. You can only interact with the "Generate SoundBanks" window.
		// If you choose to use this, comment the rest of the function.
		//GEditor->EditorAddModalWindow(WidgetWindow);

		// This creates a window that still allows you to interact with the rest of the editor. If there is an attempt to delete
		// a UAkAudioBank (from the content browser) while this window is opened, the editor will generate a (cryptic) error message
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		if (ParentWindow.IsValid())
		{
			// Parent the window to the main frame 
			FSlateApplication::Get().AddWindowAsNativeChild(WidgetWindow, ParentWindow.ToSharedRef());
		}
		else
		{
			// Spawn new window
			FSlateApplication::Get().AddWindow(WidgetWindow);
		}
	}

	void CreateClearSoundDataWindow()
	{
		if (AkAssetDatabase::Get().CheckIfLoadingAssets())
		{
			return;
		}

		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("AkAudioGenerateSoundData", "Clear Sound Data"))
			.SupportsMaximize(false).SupportsMinimize(false)
			.SizingRule(ESizingRule::Autosized)
			.FocusWhenFirstShown(true);

		TSharedRef<SClearSoundData> WindowContent = SNew(SClearSoundData);

		// Add our SClearSoundData to the window
		WidgetWindow->SetContent(WindowContent);

		// Set focus to our SClearSoundData widget, so our keyboard keys work right off the bat
		WidgetWindow->SetWidgetToFocusOnActivate(WindowContent);

		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		if (ParentWindow.IsValid())
		{
			// Parent the window to the main frame 
			FSlateApplication::Get().AddWindowAsNativeChild(WidgetWindow, ParentWindow.ToSharedRef());
		}
		else
		{
			// Spawn new window
			FSlateApplication::Get().AddWindow(WidgetWindow);
		}
	}

	void DoClearSoundData(AkSoundDataClearFlags ClearFlags)
	{

		WwiseProjectInfo wwiseProjectInfo;
		wwiseProjectInfo.Parse();

		auto start = FPlatformTime::Cycles64();

		FScopedSlowTask SlowTask(0.f, LOCTEXT("AK_ClearingSoundData", "Clearing Wwise Sound Data..."));
		SlowTask.MakeDialog();

		TArray<FString> clearCommands;
		if ((ClearFlags & AkSoundDataClearFlags::SoundBankInfoCache) == AkSoundDataClearFlags::SoundBankInfoCache)
		{
			SlowTask.EnterProgressFrame(0.f, LOCTEXT("AK_ClearSoundBankInfoCache", "Clearing SoundBankInfoCache.dat"));

			auto soundBankInfoCachePath = FPaths::Combine(wwiseProjectInfo.GetCacheDirectory(), TEXT("SoundBankInfoCache.dat"));
			FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*soundBankInfoCachePath);

			clearCommands.Add(TEXT("SoundBank Info Cache"));
		}

		if ((ClearFlags & AkSoundDataClearFlags::MediaCache) == AkSoundDataClearFlags::MediaCache)
		{
			auto cacheFolderPath = wwiseProjectInfo.GetCacheDirectory();
			TArray<FString> foldersInCache;

			auto& platformFile = FPlatformFileManager::Get().GetPlatformFile();
			platformFile.IterateDirectory(*cacheFolderPath, [&foldersInCache](const TCHAR* Path, bool IsDir) {
				if (IsDir)
				{
					foldersInCache.Add(Path);
				}

				return true;
			});

			for (auto& folder : foldersInCache)
			{
				SlowTask.EnterProgressFrame(0.f, FText::FormatOrdered(LOCTEXT("AK_ClearAssetData", "Clearing media cache {0}"), FText::FromString(folder)));

				platformFile.DeleteDirectoryRecursively(*folder);
			}

			clearCommands.Add(TEXT("Media Cache"));
		}

		auto end = FPlatformTime::Cycles64();

		UE_LOG(LogAudiokineticTools, Display, TEXT("Clear Wwise Sound Data(%s) took %f seconds."), *FString::Join(clearCommands, TEXT(", ")), FPlatformTime::ToSeconds64(end - start));
	}
}

#undef LOCTEXT_NAMESPACE
