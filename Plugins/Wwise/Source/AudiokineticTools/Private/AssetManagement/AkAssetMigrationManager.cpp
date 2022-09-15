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

#include "AkAssetMigrationManager.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkUnrealEditorHelper.h"
#include "AkAssetMigrationHelper.h"
#include "AkAudioBank.h"
#include "AkAudioEvent.h"
#include "AkAudioModule.h"
#include "AkSettingsPerUser.h"
#include "AkWaapiClient.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformFilemanager.h"
#include "IAudiokineticTools.h"
#include "Misc/FileHelper.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Settings/ProjectPackagingSettings.h"
#include "AkUnrealHelper.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "AkAudio"

bool AkAssetMigrationManager::IsMigrationRequired(AkAssetMigration::FMigrationOptions& MigrationOptions )
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (!AkSettings)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("Could not find Ak Settings, migration will not proceed"));
		return false;
	}
	auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	auto& AssetRegistry = AssetRegistryModule.Get();

	//Check if assets migration is needed
	bool bUpdateAkSettings = false;
	if (AkSettings->bAssetsMigrated ==false )
	{
		WwiseAssetsInProject.Empty();
		AkAssetMigration::FindWwiseAssetsInProject(WwiseAssetsInProject);

		if (WwiseAssetsInProject.Num() ==0)
		{
			AkSettings->bAssetsMigrated =true;
			bUpdateAkSettings= true;
		}
		else
		{
			MigrationOptions.bAssetsNotMigrated =true;
		}
	}
	else
	{
		MigrationOptions.bAssetsNotMigrated =false;
	}

	//Check if project migration is needed
	if (AkSettings->bProjectMigrated ==false )
	{
		if (AkSettings->UseEventBasedPackaging || AkSettings->SplitMediaPerFolder || IsSoundDataPathInDirectoriesToAlwaysStage(AkUnrealEditorHelper::GetLegacySoundBankDirectory()) )
		{
			// project migration needed
			MigrationOptions.bProjectSettingsNotUpToDate =true;
		}
		else
		{
			AkSettings->bProjectMigrated =true;
			bUpdateAkSettings= true;
		}
	}
	else
	{
		MigrationOptions.bProjectSettingsNotUpToDate = false;
	}

	TArray<FAssetData> Banks;
	AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetFName(), Banks);
	if (Banks.Num() > 0)
	{
		MigrationOptions.bBanksInProject = true;
	}
	else
	{
		MigrationOptions.bBanksInProject =false;
		AkSettings->bSoundBanksTransfered = true;
		bUpdateAkSettings =true;
	}

	AkAssetMigration::FindDeprecatedAssets(DeprecatedAssetsInProject);
	MigrationOptions.NumDeprecatedAssetsInProject = DeprecatedAssetsInProject.Num();
	if (DeprecatedAssetsInProject.Num() > 0)
	{
		MigrationOptions.bDeprecatedAssetsInProject = true;
	}
	else
	{
		MigrationOptions.bDeprecatedAssetsInProject = false;
	}

	if (bUpdateAkSettings)
	{
		AkUnrealEditorHelper::SaveConfigFile(AkSettings);
	}

	if ((MigrationOptions.bBanksInProject || !AkSettings->bSoundBanksTransfered) || MigrationOptions.bDeprecatedAssetsInProject  || MigrationOptions.bAssetsNotMigrated || MigrationOptions.bProjectSettingsNotUpToDate)
	{
		return true;
	}
	return false;
}

void AkAssetMigrationManager::TryMigration()
{
	AkAssetMigration::FMigrationOptions MigrationOptions;
	if (!IsMigrationRequired(MigrationOptions))
	{
		RemoveMigrationMenuOption();
		return;
	}

	AkAssetMigration::FMigrationOperations MigrationOperations;
	AkAssetMigration::PromptMigration(MigrationOptions, MigrationOperations);
	if (MigrationOperations.bCancelled)
	{
		return;
	}

	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	bool bShouldUpdateConfig = false;

	const bool bWasUsingEBP = AkSettings->UseEventBasedPackaging;
	if (MigrationOperations.bDoProjectUpdate)
	{
		if (!bWasUsingEBP)
		{
			AkSettings->RemoveSoundDataFromAlwaysStageAsUFS(AkSettings->WwiseSoundDataFolder.Path);
			AkUnrealEditorHelper::DeleteLegacySoundBanks();
		}
		else
		{
			AkSettings->RemoveSoundDataFromAlwaysCook(FString::Printf(TEXT("/Game/%s"), *AkSettings->WwiseSoundDataFolder.Path));
		}

		MigrateProjectSettings(bWasUsingEBP, AkSettings->SplitMediaPerFolder);

		AkSettings->SplitMediaPerFolder = false;
		AkSettings->UseEventBasedPackaging = false;

		bShouldUpdateConfig = true;
		AkSettings->bProjectMigrated = true;
	}


	if (MigrationOperations.BankTransferMethod != AkAssetMigration::EBankTransferMode::NoTransfer || MigrationOperations.bDoBankCleanup || MigrationOperations.bTransferAutoload)
	{
		const bool bSuccess = AkAssetMigration::MigrateAudioBanks(MigrationOperations.BankTransferMethod, MigrationOperations.bDoBankCleanup, bWasUsingEBP, MigrationOperations.bTransferAutoload);
		if (bSuccess)
		{
			AkSettings->bSoundBanksTransfered = true;
			bShouldUpdateConfig = true;
		}
	}

	if (MigrationOperations.bDoAssetMigration)
	{
		if (AkAssetMigration::MigrateWwiseAssets(WwiseAssetsInProject, AkSettings->SplitSwitchContainerMedia))
		{
			AkSettings->bAssetsMigrated = true;
			bShouldUpdateConfig = true;
		}
	}

	if (MigrationOperations.bDoDeprecatedAssetCleanup)
	{
		AkAssetMigration::DeleteDeprecatedAssets(DeprecatedAssetsInProject);
		DeprecatedAssetsInProject.Empty();
	}

	if (bShouldUpdateConfig)
	{
		AkUnrealEditorHelper::SaveConfigFile(AkSettings);
	}

	FNotificationInfo Info(LOCTEXT("AkAssetManagementManager", " Migration completed!"));
	Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkPickerTabIcon"));
	Info.bFireAndForget = true;
	Info.FadeOutDuration = 0.6f;
	Info.ExpireDuration = 4.6f;
	FSlateNotificationManager::Get().AddNotification(Info);

	GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
}


bool AkAssetMigrationManager::IsSoundDataPathInDirectoriesToAlwaysStage(const FString& SoundDataPath)
{
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	for (int32 i = PackagingSettings->DirectoriesToAlwaysStageAsUFS.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysStageAsUFS[i].Path == SoundDataPath)
		{
			return true;
		}
	}
	for (int32 i = PackagingSettings->DirectoriesToAlwaysCook.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysCook[i].Path == SoundDataPath)
		{
			return true;
		}
	}
	return false;
}


void AkAssetMigrationManager::CreateMigrationMenuOption()
{
	// Extend the build menu to handle Audiokinetic-specific entries
#if UE_5_0_OR_LATER
	{
		UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		FToolMenuSection& WwiseBuildSection = BuildMenu->AddSection(MigrationMenuSectionName, LOCTEXT("AkBuildLabel", "Audiokinetic Migration"), FToolMenuInsert("LevelEditorGeometry", EToolMenuInsertType::Default));
		FUIAction MigrationUIAction;
		MigrationUIAction.ExecuteAction.BindRaw(this, &AkAssetMigrationManager::TryMigration);
		WwiseBuildSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("AKAudioBank_PostMigration", "Finish Project Migration"),
			LOCTEXT("AkAudioBank_PostMigrationTooltip", "Transfer Bank hierarchy to Wwise, clean up bank files, delete Wwise media assets, clean up Wwise assets"),
			FSlateIcon(),
			MigrationUIAction
		);
	}
#else
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelViewportToolbarBuildMenuExtenderAkMigration = FLevelEditorModule::FLevelEditorMenuExtender::CreateLambda([this](const TSharedRef<FUICommandList> CommandList)
		{
			TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
			Extender->AddMenuExtension("LevelEditorGeometry", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.BeginSection("Audiokinetic Migration", LOCTEXT("AudiokineticMigration", "Audiokinetic Migration"));
					{
						FUIAction MigrationAction;
						MigrationAction.ExecuteAction.BindRaw(this, &AkAssetMigrationManager::TryMigration);
						MenuBuilder.AddMenuEntry(
							LOCTEXT("AKAudioBank_PostMigration", "Finish Project Migration"),
							LOCTEXT("AkAudioBank_PostMigrationTooltip", "Transfer Bank hierarchy to Wwise, clean up bank files, delete Wwise media assets, clean up Wwise assets"),
							FSlateIcon(),
							MigrationAction
						);
					}
					MenuBuilder.EndSection();
				}));

			return Extender.ToSharedRef();
		});

	LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(LevelViewportToolbarBuildMenuExtenderAkMigration);
	LevelViewportToolbarBuildMenuExtenderAkMigrationHandle = LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Last().GetHandle();
#endif
}

void AkAssetMigrationManager::RemoveMigrationMenuOption()
{
#if UE_5_0_OR_LATER
	UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
	BuildMenu->RemoveSection(MigrationMenuSectionName);
#else
	if (LevelViewportToolbarBuildMenuExtenderAkMigrationHandle.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender)
				{
					return Extender.GetHandle() == LevelViewportToolbarBuildMenuExtenderAkMigrationHandle;
				});
		}
		LevelViewportToolbarBuildMenuExtenderAkMigrationHandle.Reset();
	}
#endif
}

void AkAssetMigrationManager::ClearSoundBanksForMigration()
{
	auto soundBankDirectory = AkUnrealEditorHelper::GetLegacySoundBankDirectory();

	TArray<FString> foundFiles;

	auto& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	platformFile.FindFilesRecursively(foundFiles, *soundBankDirectory, TEXT(".bnk"));
	platformFile.FindFilesRecursively(foundFiles, *soundBankDirectory, TEXT(".json"));

	if (foundFiles.Num() > 0)
	{
		platformFile.DeleteDirectoryRecursively(*AkUnrealEditorHelper::GetLegacySoundBankDirectory());
	}
}


// This whole hack is because Unreal XML classes doesn't
// handle <!CDATA[]> which the Wwise project file use.
// Doing it the dirty way instead.
void AkAssetMigrationManager::MigrateProjectSettings(const bool& bWasUsingEBP, const bool& bUseGeneratedSubFolders)
{
	const auto ProjectPath = AkUnrealHelper::GetWwiseProjectPath();
	FString ProjectContent;
	if (FFileHelper::LoadFileToString(ProjectContent, *ProjectPath))
	{
		bool bModified = AkAssetMigration::MigrateProjectSettings(ProjectContent, bWasUsingEBP, bUseGeneratedSubFolders);
		bModified |= AkAssetMigration::SetStandardSettings(ProjectContent);

		if (bModified)
		{
			FFileHelper::SaveStringToFile(ProjectContent, *ProjectPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		}

		SetGeneratedSoundBanksPath(ProjectContent);
	}
}


void AkAssetMigrationManager::SetStandardProjectSettings()
{
	const auto ProjectPath = AkUnrealHelper::GetWwiseProjectPath();
	FString ProjectContent;
	if (FFileHelper::LoadFileToString(ProjectContent, *ProjectPath))
	{
		const bool bModified = AkAssetMigration::SetStandardSettings(ProjectContent);

		if (bModified)
		{
			FFileHelper::SaveStringToFile(ProjectContent, *ProjectPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		}
	}
}

// This whole hack is because Unreal XML classes doesn't
// handle <!CDATA[]> which the Wwise project file use.
// Doing it the dirty way instead.
bool AkAssetMigrationManager::SetGeneratedSoundBanksPath(const FString& ProjectContent)
{
	const auto SoundBankPathPosition = ProjectContent.Find(TEXT("SoundBankHeaderFilePath"));
	if (SoundBankPathPosition != INDEX_NONE)
	{
		const FString ValueDelimiter = TEXT("Value=\"");
		const auto SoundBankPathValueStartPosition = ProjectContent.Find(*ValueDelimiter, ESearchCase::IgnoreCase, ESearchDir::FromStart, SoundBankPathPosition) + ValueDelimiter.Len();
		if (SoundBankPathValueStartPosition != INDEX_NONE)
		{
			const auto SoundBankPathValueEndPosition = ProjectContent.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, SoundBankPathValueStartPosition);

			if (SoundBankPathValueEndPosition != INDEX_NONE)
			{
				auto GeneratedPath = ProjectContent.Mid(SoundBankPathValueStartPosition, SoundBankPathValueEndPosition - SoundBankPathValueStartPosition);
				if(FPaths::IsRelative(GeneratedPath))
				{
					auto WwiseProjectDirectory = FPaths::GetPath(AkUnrealHelper::GetWwiseProjectPath());
					GeneratedPath = FPaths::Combine(WwiseProjectDirectory, GeneratedPath);
				}
				FPaths::MakePathRelativeTo(GeneratedPath, *FPaths::ProjectContentDir());
				UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
				if (AkSettings)
				{
					AkSettings->GeneratedSoundBanksFolder.Path = GeneratedPath;
					FAkAudioModule::AkAudioModuleInstance->UpdateWwiseResourceLoaderSettings();
					if (UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>())
					{
						UserSettings->OnGeneratedSoundBanksPathChanged.Broadcast();
					}
					return true;
				}
			}
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE
