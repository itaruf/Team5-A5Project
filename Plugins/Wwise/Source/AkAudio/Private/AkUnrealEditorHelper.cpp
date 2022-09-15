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

#include "AkUnrealEditorHelper.h"



#if WITH_EDITOR

#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/MessageDialog.h"
#include "ISourceControlModule.h"
#include "SSettingsEditorCheckoutNotice.h"

#include "AkUnrealHelper.h"
#include "AkSettings.h"

#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

namespace AkUnrealEditorHelper
{

	const TCHAR* LocalizedFolderName = TEXT("Localized");


	void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
	{
		AkUnrealHelper::TrimPath(Path);

		FText FailReason;
		if (!FPaths::ValidatePath(Path, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			Path = PreviousPath;
			return;
		}

		const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Path);
		if (!FPaths::DirectoryExists(AbsolutePath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
			Path = PreviousPath;
			return;
		}
	}

	bool SanitizeFolderPathAndMakeRelativeToContentDir(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
	{
		AkUnrealHelper::TrimPath(Path);

		FString TempPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*Path);

		FText FailReason;
		if (!FPaths::ValidatePath(TempPath, &FailReason))
		{
			if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, FailReason))
			{
				Path = PreviousPath;
				return false;
			}
		}

		auto ContentDirectory = AkUnrealHelper::GetContentDirectory();
		if (!FPaths::FileExists(TempPath))
		{
			// Path might be a valid one (relative to game) entered manually. Check that.
			TempPath = FPaths::ConvertRelativePathToFull(ContentDirectory, Path);

			if (!FPaths::DirectoryExists(TempPath))
			{
				if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, DialogMessage))
				{
					Path = PreviousPath;
					return false;
				}
			}
		}

		// Make the path relative to the game dir
		FPaths::MakePathRelativeTo(TempPath, *ContentDirectory);
		Path = TempPath;

		if (Path != PreviousPath)
		{

			return true;
		}
		return false;
	}

	void SaveConfigFile(UObject* ConfigObject)
	{
		FString ConfigFilename = ConfigObject->GetDefaultConfigFilename();
		if (!ISourceControlModule::Get().IsEnabled() || SettingsHelpers::IsCheckedOut(ConfigFilename) || SettingsHelpers::CheckOutOrAddFile(ConfigFilename))
		{
#if UE_5_0_OR_LATER
			ConfigObject->TryUpdateDefaultConfigFile();
#else
			ConfigObject->UpdateDefaultConfigFile();
#endif
		}
	}

	FString GetLegacySoundBankDirectory()
	{
		if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
		{
			return FPaths::Combine(AkUnrealHelper::GetContentDirectory(), AkSettings->WwiseSoundDataFolder.Path);
		}
		else
		{
			return FPaths::Combine(AkUnrealHelper::GetContentDirectory(), UAkSettings::DefaultSoundDataFolder);
		}
	}

	FString GetContentDirectory()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	}


#define LOCTEXT_NAMESPACE "AkAudio"
	void DeleteLegacySoundBanks()
	{
		const TArray<FString> ExtensionsToDelete = { "bnk", "wem", "json", "txt", "xml" };
		bool SuccessfulDelete = true;
		for (auto& Extension : ExtensionsToDelete)
		{
			TArray<FString> FoundFiles;
			FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(FoundFiles, *AkUnrealHelper::GetSoundBankDirectory(), *Extension);
			FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(FoundFiles, *GetLegacySoundBankDirectory(), *Extension);
			for (auto& File : FoundFiles)
			{
				SuccessfulDelete = FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*File);
			}
		}

		if (!SuccessfulDelete)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CannotDeleteOldBanks", "Unable to delete legacy SoundBank files. Please ensure to manually delete them after migration is complete."));
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CannotDeleteOldBanks", "Unable to delete all legacy SoundBank files. Please ensure to manually delete them after migration is complete."));
		}
	}
#undef LOCTEXT_NAMESPACE

}

#endif		// WITH_EDITOR