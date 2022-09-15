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


/*=============================================================================
	AudiokineticToolsModule.cpp
=============================================================================*/
#include "AudiokineticToolsPrivatePCH.h"

#include "AkAcousticPortal.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioStyle.h"
#include "AkComponent.h"
#include "AkEventAssetBroker.h"
#include "AkGeometryComponent.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkSurfaceReflectorSetComponent.h"
#include "AkUnrealHelper.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "AssetManagement/AkAssetMigrationManager.h"
#include "AssetManagement/AkGenerateSoundBanksTask.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ComponentAssetBroker.h"
#include "ContentBrowserModule.h"
#include "DetailsCustomization/AkGeometryComponentDetailsCustomization.h"
#include "DetailsCustomization/AkLateReverbComponentDetailsCustomization.h"
#include "DetailsCustomization/AkRoomComponentDetailsCustomization.h"
#include "DetailsCustomization/AkPortalComponentDetailsCustomization.h"
#include "DetailsCustomization/AkSurfaceReflectorSetDetailsCustomization.h"
#include "DetailsCustomization/AkSettingsDetailsCustomization.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Editor/UnrealEdEngine.h"
#include "Factories/ActorFactoryAkAmbientSound.h"
#include "Factories/AkAssetTypeActions.h"
#include "Framework/Application/SlateApplication.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Interfaces/IProjectManager.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "IAudiokineticTools.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "MovieScene.h"
#include "Platforms/AkUEPlatform.h"
#include "ProjectDescriptor.h"
#include "PropertyEditorModule.h"
#include "Sequencer/MovieSceneAkAudioEventTrackEditor.h"
#include "Sequencer/MovieSceneAkAudioRTPCTrackEditor.h"
#include "Settings/ProjectPackagingSettings.h"
#include "SettingsEditor/Public/ISettingsEditorModule.h"
#include "AkUnrealEditorHelper.h"
#include "UnrealEd/Public/EditorBuildUtils.h"
#include "UnrealEdGlobals.h"
#include "UnrealEdMisc.h"
#include "Visualizer/AkAcousticPortalVisualizer.h"
#include "Visualizer/AkComponentVisualizer.h"
#include "Visualizer/AkSurfaceReflectorSetComponentVisualizer.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SSpacer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "AssetManagement/GeneratedSoundBanksDirectoryWatcher.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "WwisePicker/SWwisePickerGroup.h"
#include "WwiseProject/AcousticTextureParamLookup.h"
#include "Developer/ToolMenus/Public/ToolMenu.h"
#include "Developer/ToolMenus/Public/ToolMenus.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"
#include "AkAudioModule.h"
#include "AssetManagement/WaapiRenameWatcher.h"

#define LOCTEXT_NAMESPACE "AkAudio"
DEFINE_LOG_CATEGORY(LogAudiokineticTools);

namespace
{
	struct WwiseLanguageToUnrealCulture
	{
		const TCHAR* WwiseLanguage;
		const TCHAR* UnrealCulture;
	};

	// This list come from the fixed list of languages that were used before Wwise 2017.1
	const WwiseLanguageToUnrealCulture WwiseLanguageToUnrealCultureList[] = {
		{TEXT("Arabic"), TEXT("ar")},
		{TEXT("Bulgarian"), TEXT("bg")},
		{TEXT("Chinese(HK)"), TEXT("zh-HK")},
		{TEXT("Chinese(Malaysia)"), TEXT("zh")},
		{TEXT("Chinese(PRC)"), TEXT("zh-CN")},
		{TEXT("Chinese(Taiwan)"), TEXT("zh-TW")},
		{TEXT("Czech"), TEXT("cs")},
		{TEXT("Danish"), TEXT("da")},
		{TEXT("English(Australia)"), TEXT("en-AU")},
		{TEXT("English(Canada)"), TEXT("en-CA")},
		{TEXT("English(US)"), TEXT("en-US")},
		{TEXT("English(UK)"), TEXT("en-GB")},
		{TEXT("Finnish"), TEXT("fi")},
		{TEXT("French(Canada)"), TEXT("fr-CA")},
		{TEXT("French(France)"), TEXT("fr-FR")},
		{TEXT("German"), TEXT("de")},
		{TEXT("Greek"), TEXT("el")},
		{TEXT("Hebrew"), TEXT("he")},
		{TEXT("Hungarian"), TEXT("hu")},
		{TEXT("Indonesian"), TEXT("id")},
		{TEXT("Italian"), TEXT("it")},
		{TEXT("Japanese"), TEXT("ja")},
		{TEXT("Korean"), TEXT("ko")},
		{TEXT("Norwegian "), TEXT("no")},
		{TEXT("Polish"), TEXT("pl")},
		{TEXT("Portuguese(Brazil)"), TEXT("pt-BR")},
		{TEXT("Portuguese(Portugal)"), TEXT("pt-PT")},
		{TEXT("Romanian"), TEXT("ro")},
		{TEXT("Russian"), TEXT("ru")},
		{TEXT("Slovenian"), TEXT("sl")},
		{TEXT("Spanish(Mexico)"), TEXT("es-MX")},
		{TEXT("Spanish(Spain)"), TEXT("es-ES")},
		{TEXT("Swedish"), TEXT("sv")},
		{TEXT("Thai"), TEXT("th")},
		{TEXT("Turkish"), TEXT("tr")},
		{TEXT("Ukrainian"), TEXT("uk")},
		{TEXT("Vietnamese"), TEXT("vi")},
	};
}

class FAudiokineticToolsModule : public IAudiokineticTools
{
	/**
 * Creates a new WwisePickerGroup tab.
 *
 * @param SpawnTabArgs The arguments for the tab to spawn.
 * @return The spawned tab.
 */
	TSharedRef<SDockTab> CreateWwisePickerGroupTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		const TSharedRef<SDockTab> DockTab = SNew(SDockTab)
			.TabRole(ETabRole::NomadTab);

		TSharedRef<SWwisePickerGroup> PickerGroup = SNew(SWwisePickerGroup, DockTab, SpawnTabArgs.GetOwnerWindow());
		WeakPickerGroup = PickerGroup;

		DockTab->SetContent(PickerGroup);

		return DockTab;
	}

	void RefreshWwiseProject() override
	{
		SoundBanksDirectoryWatcher.RestartWatchers();
		if (auto* ProjectDatabase = UWwiseProjectDatabase::Get())
		{
			ProjectDatabase->UpdateDataStructure();
		}
	}

	void OpenOnlineHelp()
	{
		FPlatformProcess::LaunchFileInDefaultExternalApplication(TEXT("https://www.audiokinetic.com/library/?source=UE4&id=index.html"));
	}

	static void ToggleVisualizeRoomsAndPortals()
	{
		UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
		if (AkSettings != nullptr)
		{
			AkSettings->ToggleVisualizeRoomsAndPortals();
		}
	}

	static bool IsVisualizeRoomsAndPortalsEnabled()
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings == nullptr)
			return false;
		return AkSettings->VisualizeRoomsAndPortals;
	}

	static ECheckBoxState GetVisualizeRoomsAndPortalsCheckBoxState()
	{
		return IsVisualizeRoomsAndPortalsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	static void ToggleShowReverbInfo()
	{
		UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
		if (AkSettings != nullptr)
		{
			AkSettings->ToggleShowReverbInfo();
		}
	}

	static bool IsReverbInfoEnabled()
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings == nullptr)
			return false;
		return AkSettings->bShowReverbInfo;
	}

	static ECheckBoxState GetReverbInfoCheckBoxState()
	{
		return IsReverbInfoEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void CreateAkViewportCommands()
	{
		// Extend the viewport menu and add the Audiokinetic commands
		{
			UToolMenu* ViewportMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportToolBar.Options");
			FToolMenuSection& AkSection = ViewportMenu->AddSection("Audiokinetic", LOCTEXT("AkLabel", "Audiokinetic"), FToolMenuInsert("Audiokinetic", EToolMenuInsertType::Default));

			ToggleVizRoomsPortalsAction.ExecuteAction.BindStatic(&FAudiokineticToolsModule::ToggleVisualizeRoomsAndPortals);
			ToggleVizRoomsPortalsAction.GetActionCheckState.BindStatic(&FAudiokineticToolsModule::GetVisualizeRoomsAndPortalsCheckBoxState);

			AkSection.AddMenuEntry(
				NAME_None,
				LOCTEXT("ToggleVizRoomsAndPortals_Label", "Visualize Rooms And Portals"),
				LOCTEXT("ToggleVizRoomsAndPortals_Tip", "Toggles the visualization of rooms and portals in the viewport. This requires 'realtime' to be enabled in the viewport."),
				FSlateIcon(),
				ToggleVizRoomsPortalsAction,
				EUserInterfaceActionType::ToggleButton
			);

			ToggleReverbInfoAction.ExecuteAction.BindStatic(&FAudiokineticToolsModule::ToggleShowReverbInfo);
			ToggleReverbInfoAction.GetActionCheckState.BindStatic(&FAudiokineticToolsModule::GetReverbInfoCheckBoxState);

			AkSection.AddMenuEntry(
				NAME_None,
				LOCTEXT("ToggleReverbInfo_Label", "Show Reverb Info"),
				LOCTEXT("ToggleReverbInfo_Tip", "When enabled, information about AkReverbComponents will be displayed in viewports, above the component's UPrimitiveComponent parent. This requires 'realtime' to be enabled in the viewport."),
				FSlateIcon(),
				ToggleReverbInfoAction,
				EUserInterfaceActionType::ToggleButton
			);
		}
	}

	void RegisterWwiseMenus()
	{
		// Extend the build menu to handle Audiokinetic-specific entries
#if UE_5_0_OR_LATER
		{
			UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
			FToolMenuSection& WwiseBuildSection = BuildMenu->AddSection("AkBuild", LOCTEXT("AkBuildLabel", "Audiokinetic"), FToolMenuInsert("LevelEditorGeometry", EToolMenuInsertType::Default));

			FUIAction GenerateSoundDataUIAction;
			GenerateSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow, false);
			WwiseBuildSection.AddMenuEntry(
				NAME_None,
				LOCTEXT("AkAudioBank_GenerateSoundBanks", "Generate SoundBanks..."),
				LOCTEXT("AkAudioBank_GenerateSoundBanksTooltip", "Generates Wwise SoundBanks."),
				FSlateIcon(),
				GenerateSoundDataUIAction
			);

			FUIAction RefreshProjectUIAction;
			RefreshProjectUIAction.ExecuteAction.BindRaw(this, &FAudiokineticToolsModule::RefreshWwiseProject);
			WwiseBuildSection.AddMenuEntry(
				NAME_None,
				LOCTEXT("RefreshWwiseProject", "Refresh Project Database"),
				LOCTEXT("RefreshWwiseProjectTooltip", "Reparse the the Wwise Project in GeneratedSoundBanks and reload Wwise assets."),
				FSlateIcon(),
				RefreshProjectUIAction
			);

			FUIAction ClearSoundDataUIAction;
			ClearSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateClearSoundDataWindow);
			WwiseBuildSection.AddMenuEntry(
				NAME_None,
				LOCTEXT("AkAudioBank_ClearSoundData", "Clear Wwise Cache..."),
				LOCTEXT("AkAudioBank_ClearSoundDataTooltip", "Clear the Wwise cache folder."),
				FSlateIcon(),
				ClearSoundDataUIAction
			);
		}
#else
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		LevelViewportToolbarBuildMenuExtenderAk = FLevelEditorModule::FLevelEditorMenuExtender::CreateLambda([this](const TSharedRef<FUICommandList> CommandList)
			{
				TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
				Extender->AddMenuExtension("LevelEditorGeometry", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
					{
						MenuBuilder.BeginSection("Audiokinetic", LOCTEXT("Audiokinetic", "Audiokinetic"));
						{
							FUIAction GenerateSoundDataUIAction;
							GenerateSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow, false);
							MenuBuilder.AddMenuEntry(
								LOCTEXT("AkAudioBank_GenerateSoundBanks", "Generate SoundBanks..."),
								LOCTEXT("AkAudioBank_GenerateSoundBanksTooltip", "Generates Wwise SoundBanks."),
								FSlateIcon(),
								GenerateSoundDataUIAction
							);

							FUIAction RefreshProjectUIAction;
							RefreshProjectUIAction.ExecuteAction.BindRaw(this, &FAudiokineticToolsModule::RefreshWwiseProject);
							MenuBuilder.AddMenuEntry(
								LOCTEXT("AkAudioBank_RefreshProject", "Refresh Project"),
								LOCTEXT("AkAudioBank_RefreshProjectTooltip", "Refresh the Wwise Project"),
								FSlateIcon(),
								RefreshProjectUIAction
							);

							FUIAction ClearSoundDataUIAction;
							ClearSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateClearSoundDataWindow);
							MenuBuilder.AddMenuEntry(
								LOCTEXT("AkAudioBank_ClearSoundData", "Clear Wwise Cache..."),
								LOCTEXT("AkAudioBank_ClearSoundDataTooltip", "Clear the Wwise cache folder."),
								FSlateIcon(),
								ClearSoundDataUIAction
							);
						}
						MenuBuilder.EndSection();

					}));

				return Extender.ToSharedRef();
			});
		LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(LevelViewportToolbarBuildMenuExtenderAk);
		LevelViewportToolbarBuildMenuExtenderAkHandle = LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Last().GetHandle();
#endif

		// Extend the Help menu to display a link to our documentation
		{
			UToolMenu* HelpMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help");
			FToolMenuSection& WwiseHelpSection = HelpMenu->AddSection("AkHelp", LOCTEXT("AkHelpLabel", "Audiokinetic"), FToolMenuInsert("HelpBrowse", EToolMenuInsertType::Default));

			WwiseHelpSection.AddEntry(FToolMenuEntry::InitMenuEntry(
				NAME_None,
				LOCTEXT("AkWwiseHelpEntry", "Wwise Help"),
				LOCTEXT("AkWwiseHelpEntryToolTip", "Shows the online Wwise documentation."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FAudiokineticToolsModule::OpenOnlineHelp))
			));
		}
	}

	void UpdateUnrealCultureToWwiseCultureMap(const WwiseProjectInfo& wwiseProjectInfo)
	{
		static constexpr auto InvariantCultureLCID = 0x007F;

		UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
		if (!AkSettings)
		{
			return;
		}

		TMap<FString, FString> wwiseToUnrealMap;
		for (auto& entry : WwiseLanguageToUnrealCultureList)
		{
			wwiseToUnrealMap.Add(entry.WwiseLanguage, entry.UnrealCulture);
		}

		TMap<FString, int> languageCountMap;

		for (auto& language : wwiseProjectInfo.GetSupportedLanguages())
		{
			if (auto* foundUnrealCulture = wwiseToUnrealMap.Find(language.Name))
			{
				auto culturePtr = FInternationalization::Get().GetCulture(*foundUnrealCulture);

				if (culturePtr && culturePtr->GetLCID() != InvariantCultureLCID)
				{
					int& langCount = languageCountMap.FindOrAdd(culturePtr->GetTwoLetterISOLanguageName());
					++langCount;
				}
			}
		}

		TSet<FString> foundCultures;

		bool modified = false;
		for (auto& language : wwiseProjectInfo.GetSupportedLanguages())
		{
			if (auto* foundUnrealCulture = wwiseToUnrealMap.Find(language.Name))
			{
				auto culturePtr = FInternationalization::Get().GetCulture(*foundUnrealCulture);

				if (culturePtr && culturePtr->GetLCID() != InvariantCultureLCID)
				{
					int* langCount = languageCountMap.Find(culturePtr->GetTwoLetterISOLanguageName());

					if (langCount && *langCount > 1)
					{
						auto newKey = *foundUnrealCulture;
						if (!AkSettings->UnrealCultureToWwiseCulture.Contains(newKey))
						{
							AkSettings->UnrealCultureToWwiseCulture.Add(newKey, language.Name);
							modified = true;
						}

						foundCultures.Add(newKey);
					}
					else
					{
						auto newKey = culturePtr->GetTwoLetterISOLanguageName();
						if (!AkSettings->UnrealCultureToWwiseCulture.Contains(newKey))
						{
							AkSettings->UnrealCultureToWwiseCulture.Add(newKey, language.Name);
							modified = true;
						}

						foundCultures.Add(newKey);
					}
				}
			}
			else
			{
				for (auto& entry : AkSettings->UnrealCultureToWwiseCulture)
				{
					if (entry.Value == language.Name)
					{
						foundCultures.Add(entry.Key);
						break;
					}
				}
			}
		}

		TSet<FString> keysToRemove;
		for (auto& entry : AkSettings->UnrealCultureToWwiseCulture)
		{
			if (!foundCultures.Contains(entry.Key))
			{
				keysToRemove.Add(entry.Key);
			}
		}

		for (auto& keyToRemove : keysToRemove)
		{
			AkSettings->UnrealCultureToWwiseCulture.Remove(keyToRemove);
			modified = true;
		}

		if (modified)
		{
			AkSettings->SaveConfig();
		}
	}

	void VerifyWwiseProjectPath(UAkSettings* AkSettings, UAkSettingsPerUser* AkSettingsPerUser)
	{
		if (AkSettings->WwiseProjectPath.FilePath.IsEmpty())
		{
			if (!AkSettingsPerUser->SuppressWwiseProjectPathWarnings && FApp::CanEverRender())
			{
				if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SettingsNotSet", "Wwise settings do not seem to be set. Would you like to open the settings window to set them?")))
				{
					FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Wwise"), FName("User Settings"));
				}
			}
			else
			{
				UE_LOG(LogAudiokineticTools, Log, TEXT("Wwise project not found. The Wwise picker will not be usable."));
			}
		}
		else
		{
			// First-time plugin migration: Project might be relative to Engine path. Fix-up the path to make it relative to the game.
			const auto ProjectDir = FPaths::ProjectDir();
			FString FullGameDir = FPaths::ConvertRelativePathToFull(ProjectDir);
			FString TempPath = FPaths::ConvertRelativePathToFull(FullGameDir, AkSettings->WwiseProjectPath.FilePath);
			if (!FPaths::FileExists(TempPath))
			{
				if (!AkSettingsPerUser->SuppressWwiseProjectPathWarnings)
				{
					TSharedPtr<SWindow> Dialog = SNew(SWindow)
						.Title(LOCTEXT("ResetWwisePath", "Re-set Wwise Path"))
						.SupportsMaximize(false)
						.SupportsMinimize(false)
						.FocusWhenFirstShown(true)
						.SizingRule(ESizingRule::Autosized);

					TSharedRef<SWidget> DialogContent = SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.25f)
						[
							SNew(SSpacer)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AkUpdateWwisePath", "The Wwise UE4 Integration plug-in's update process requires the Wwise Project Path to be set in the Project Settings dialog. Would you like to open the Project Settings?"))
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(0.75f)
						[
							SNew(SSpacer)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SCheckBox)
							.Padding(FMargin(6.0, 2.0))
							.OnCheckStateChanged_Lambda([&](ECheckBoxState DontAskState) {
								AkSettingsPerUser->SuppressWwiseProjectPathWarnings = (DontAskState == ECheckBoxState::Checked);
							})
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AkDontShowAgain", "Don't show this again"))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SSpacer)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 3.0f, 0.0f, 3.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("Yes", "Yes"))
								.OnClicked_Lambda([&]() -> FReply {
									FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Plugins"), FName("Wwise"));
									Dialog->RequestDestroyWindow();
									AkSettingsPerUser->SaveConfig();
									return FReply::Handled();
								})
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 3.0f, 0.0f, 3.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("No", "No"))
								.OnClicked_Lambda([&]() -> FReply {
									Dialog->RequestDestroyWindow();
									AkSettingsPerUser->SaveConfig();
									return FReply::Handled();
								})
							]
						]
					;

					Dialog->SetContent(DialogContent);
					FSlateApplication::Get().AddModalWindow(Dialog.ToSharedRef(), nullptr);
				}
				else
				{
					UE_LOG(LogAudiokineticTools, Log, TEXT("Wwise project not found. The Wwise picker will not be usable."));
				}
			}
			else
			{
				FPaths::MakePathRelativeTo(TempPath, *ProjectDir);
				if (AkSettings->WwiseProjectPath.FilePath != TempPath)
				{
					AkSettings->WwiseProjectPath.FilePath = TempPath;
					AkUnrealEditorHelper::SaveConfigFile(AkSettings);
				}
			}
		}
	}

	void OnAssetRegistryFilesLoaded()
	{
		UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
		UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
		auto* CurrentProject = IProjectManager::Get().GetCurrentProject();

		bool doModifyProject = true;

		WwiseProjectInfo wwiseProjectInfo;
		wwiseProjectInfo.Parse();

		// If we're on the project loader screen, we don't want to display the dialog.
		// In that case, CurrentProject is nullptr.
		if (CurrentProject && AkSettings && AkSettingsPerUser)
		{
			VerifyWwiseProjectPath(AkSettings, AkSettingsPerUser);

			if (doModifyProject)
			{
				AssetMigrationManager.SetStandardProjectSettings();
			}


		}

		UpdateUnrealCultureToWwiseCultureMap(wwiseProjectInfo);

		if (GUnrealEd != NULL)
		{
			GUnrealEd->RegisterComponentVisualizer(UAkComponent::StaticClass()->GetFName(), MakeShareable(new FAkComponentVisualizer));
			GUnrealEd->RegisterComponentVisualizer(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName(), MakeShareable(new FAkSurfaceReflectorSetComponentVisualizer));
			GUnrealEd->RegisterComponentVisualizer(UAkPortalComponent::StaticClass()->GetFName(), MakeShareable(new UAkPortalComponentVisualizer));
		}

		AkSettings->InitAkGeometryMap();
		AkSettings->EnsurePluginContentIsInAlwaysCook();

		AkAcousticTextureParamLookup AcousticTextureParamLookup;
		AcousticTextureParamLookup.UpdateParamsMap();


		AssetMigrationManager.CreateMigrationMenuOption();
		AssetMigrationManager.TryMigration();

		SoundBanksDirectoryWatcher.Initialize();
	}

	virtual void StartupModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			auto AudiokineticAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Audiokinetic")), LOCTEXT("AudiokineticAssetCategory", "Audiokinetic"));

			AkAssetTypeActionsArray =
			{
				MakeShared<FAssetTypeActions_AkAudioEvent>(AudiokineticAssetCategoryBit),
				MakeShared<FAssetTypeActions_AkAcousticTexture>(AudiokineticAssetCategoryBit),
				MakeShared<FAssetTypeActions_AkAuxBus>(AudiokineticAssetCategoryBit),
				MakeShared<FAssetTypeActions_AkRtpc>(AudiokineticAssetCategoryBit),
				MakeShared<FAssetTypeActions_AkTrigger>(AudiokineticAssetCategoryBit),
			};

			for (auto& AkAssetTypeActions : AkAssetTypeActionsArray)
				AssetTools.RegisterAssetTypeActions(AkAssetTypeActions.ToSharedRef());
		}

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor") && !IsRunningCommandlet())
		{
			RegisterWwiseMenus();
			CreateAkViewportCommands();
		}

		RegisterSettings();

		AkEventBroker = MakeShared<FAkEventAssetBroker>();
		FComponentAssetBrokerage::RegisterBroker(AkEventBroker, UAkComponent::StaticClass(), true, true);

		auto& TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SWwisePickerGroup::WwisePickerGroupTabName, FOnSpawnTab::CreateRaw(this, &FAudiokineticToolsModule::CreateWwisePickerGroupTab))
			.SetDisplayName(NSLOCTEXT("FAudiokineticToolsModule", "PickerTabTitle", "Wwise Pickers"))
			.SetTooltipText(NSLOCTEXT("FAudiokineticToolsModule", "PickerTooltipText", "Open the Wwise Pickers tab."))
			.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
			.SetIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkPickerTabIcon"));

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		OnAssetRegistryFilesLoadedHandle = AssetRegistryModule.Get().OnFilesLoaded().AddRaw(this, &FAudiokineticToolsModule::OnAssetRegistryFilesLoaded);

		ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>(TEXT("Sequencer"));
		RTPCTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMovieSceneAkAudioRTPCTrackEditor::CreateTrackEditor));
		EventTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMovieSceneAkAudioEventTrackEditor::CreateTrackEditor));

		// Since we are initialized in the PostEngineInit phase, our Ambient Sound actor factory is not registered. We need to register it ourselves.
		if (GEditor)
		{
			if (auto NewFactory = NewObject<UActorFactoryAkAmbientSound>())
			{
				GEditor->ActorFactories.Add(NewFactory);
			}
		}

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkSurfaceReflectorSetDetailsCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UAkLateReverbComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkLateReverbComponentDetailsCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UAkRoomComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkRoomComponentDetailsCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UAkPortalComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkPortalComponentDetailsCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UAkGeometryComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkGeometryComponentDetailsCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UAkSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkSettingsDetailsCustomization::MakeInstance));

		if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
		{
			return;
		}

		AkAssetDatabase::Get().Init();
		WaapiRenameWatcher::Get().Init();
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			auto& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

			for (auto AkAssetTypeActions : AkAssetTypeActionsArray)
				if (AkAssetTypeActions.IsValid())
					AssetTools.UnregisterAssetTypeActions(AkAssetTypeActions.ToSharedRef());
		}

		AkAssetTypeActionsArray.Empty();

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender)
				{
					return Extender.GetHandle() == LevelViewportToolbarBuildMenuExtenderAkHandle;
				});

			if (MainMenuExtender.IsValid())
			{
				LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MainMenuExtender);
			}
		}
		LevelViewportToolbarBuildMenuExtenderAkHandle.Reset();

		UnregisterSettings();

		if (GUnrealEd != NULL)
		{
			GUnrealEd->UnregisterComponentVisualizer(UAkComponent::StaticClass()->GetFName());
		}

		FGlobalTabmanager::Get()->UnregisterTabSpawner(SWwisePickerGroup::WwisePickerGroupTabName);

		if (FModuleManager::Get().IsModuleLoaded(TEXT("Sequencer")))
		{
			auto& SequencerModule = FModuleManager::GetModuleChecked<ISequencerModule>(TEXT("Sequencer"));
			SequencerModule.UnRegisterTrackEditor(RTPCTrackEditorHandle);
			SequencerModule.UnRegisterTrackEditor(EventTrackEditorHandle);
		}

		// Only found way to close the tab in the case of a hot-reload. We need a pointer to the DockTab, and the only way of getting it seems to be InvokeTab.
		if (IsValid(GUnrealEd))
		{
#if UE_4_26_OR_LATER
			auto WwisePickerGroupTab = FGlobalTabmanager::Get()->TryInvokeTab(SWwisePickerGroup::WwisePickerGroupTabName);
			if (WwisePickerGroupTab.IsValid())
			{
				WwisePickerGroupTab->RequestCloseTab();
				if (WeakPickerGroup.IsValid())
				{
					WeakPickerGroup.Pin()->RequestClosePickerTabs();
				}
			}
#else
			if (WeakPickerGroup.IsValid())
			{
				WeakPickerGroup.Pin()->RequestClosePickerTabs();
			}
			FGlobalTabmanager::Get()->InvokeTab(SWwisePickerGroup::WwisePickerGroupTabName)->RequestCloseTab();
#endif
		}

		if (WeakPickerGroup.IsValid())
		{
			WeakPickerGroup.Pin()->UnregisterPickerTabs();
		}

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SWwisePickerGroup::WwisePickerGroupTabName);

		if (UObjectInitialized())
		{
			FComponentAssetBrokerage::UnregisterBroker(AkEventBroker);
		}

		auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UAkLateReverbComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UAkRoomComponent::StaticClass()->GetFName());

		if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
		{
			return;
		}

		AkAssetDatabase::Get().UnInit();
		WaapiRenameWatcher::Get().UnInit();
		SoundBanksDirectoryWatcher.Uninitialize(true);

	}

	static EEditorBuildResult BuildAkEventData(UWorld* world, FName name)
	{
		if (!AkAssetDatabase::Get().CheckIfLoadingAssets())
		{
			AkGenerateSoundBanksTask::ExecuteForEditorPlatform();
			return EEditorBuildResult::InProgress;
		}
		else
		{
			return EEditorBuildResult::Skipped;
		}
	}

private:
	struct SettingsRegistrationStruct
	{
		SettingsRegistrationStruct(UClass* SettingsClass, const FName& SectionName, const FText& DisplayName, const FText& Description)
			: SettingsClass(SettingsClass), SectionName(SectionName), DisplayName(DisplayName), Description(Description)
		{}

		void Register(ISettingsModule* SettingsModule) const
		{
			SettingsModule->RegisterSettings("Project", "Wwise", SectionName, DisplayName, Description, SettingsObject());
		}

		void Unregister(ISettingsModule* SettingsModule) const
		{
			SettingsModule->UnregisterSettings("Project", "Wwise", SectionName);
		}

	private:
		UClass* SettingsClass;
		const FName SectionName;
		const FText DisplayName;
		const FText Description;

		UObject* SettingsObject() const { return SettingsClass->GetDefaultObject(); }
	};

	static TMap<FString, SettingsRegistrationStruct>& GetWwisePlatformNameToSettingsRegistrationMap()
	{
		static TMap<FString, SettingsRegistrationStruct> WwisePlatformNameToWwiseSettingsRegistrationMap;
		if (WwisePlatformNameToWwiseSettingsRegistrationMap.Num() == 0)
		{
			auto RegisterIntegrationSettings = SettingsRegistrationStruct(UAkSettings::StaticClass(),
				"Integration",
				LOCTEXT("WwiseIntegrationSettingsName", "Integration Settings"),
				LOCTEXT("WwiseIntegrationSettingsDescription", "Configure the Wwise Integration"));

			auto RegisterPerUserSettings = SettingsRegistrationStruct(UAkSettingsPerUser::StaticClass(),
				"User Settings",
				LOCTEXT("WwiseRuntimePerUserSettingsName", "User Settings"),
				LOCTEXT("WwiseRuntimePerUserSettingsDescription", "Configure the Wwise Integration per user"));

			WwisePlatformNameToWwiseSettingsRegistrationMap.Add(FString("Integration"), RegisterIntegrationSettings);
			WwisePlatformNameToWwiseSettingsRegistrationMap.Add(FString("User"), RegisterPerUserSettings);

			for (const auto& AvailablePlatform : AkUnrealPlatformHelper::GetAllSupportedUnrealPlatforms())
			{
				FString SettingsClassName = FString::Format(TEXT("Ak{0}InitializationSettings"), { *AvailablePlatform });
				UClass* SettingsClass = FindObject<UClass>(ANY_PACKAGE, *SettingsClassName);
				if (SettingsClass)
				{
					FString CategoryNameKey = FString::Format(TEXT("Wwise{0}SettingsName"), { *AvailablePlatform });
					FString DescriptionNameKey = FString::Format(TEXT("Wwise{0}SettingsDescription"), { *AvailablePlatform });
					FString DescriptionText = FString::Format(TEXT("Configure the Wwise {0} Initialization Settings"), { *AvailablePlatform });
					FText PlatformNameText = FText::FromString(*AvailablePlatform);
					FString AdditionalDescriptionText = TEXT("");
					if (AkUnrealPlatformHelper::IsEditorPlatform(AvailablePlatform))
					{
						AdditionalDescriptionText = TEXT("\nYou must restart the Unreal Editor for changes to be applied to the Wwise Sound Engine running in the Editor");
					}
					FText PlatformDescriptionText = FText::Format(LOCTEXT("WwiseSettingsDescription", "Configure the Wwise {0} Initialization Settings{1}"), PlatformNameText, FText::FromString(*AdditionalDescriptionText));
					auto RegisterPlatform = SettingsRegistrationStruct(SettingsClass, FName(*AvailablePlatform),
						PlatformNameText,
						PlatformDescriptionText);
					WwisePlatformNameToWwiseSettingsRegistrationMap.Add(*AvailablePlatform, RegisterPlatform);
				}
			}
		}
		return WwisePlatformNameToWwiseSettingsRegistrationMap;
	}

	TSet<FString> RegisteredSettingsNames;

	void RegisterSettings()
	{
		if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			auto UpdatePlatformSettings = [SettingsModule, this]
			{
				auto SettingsRegistrationMap = GetWwisePlatformNameToSettingsRegistrationMap();

				TSet<FString> SettingsThatShouldBeRegistered = { FString("Integration"), FString("User") };

				for (const auto& AvailablePlatform : AkUnrealPlatformHelper::GetAllSupportedUnrealPlatformsForProject())
				{
					if (SettingsRegistrationMap.Contains(AvailablePlatform))
					{
						SettingsThatShouldBeRegistered.Add(AvailablePlatform);
					}
				}

				auto SettingsToBeUnregistered = RegisteredSettingsNames.Difference(SettingsThatShouldBeRegistered);
				for (const auto& SettingsName : SettingsToBeUnregistered)
				{
					SettingsRegistrationMap[SettingsName].Unregister(SettingsModule);
					RegisteredSettingsNames.Remove(SettingsName);
				}

				auto SettingsToBeRegistered = SettingsThatShouldBeRegistered.Difference(RegisteredSettingsNames);
				for (const auto& SettingsName : SettingsToBeRegistered)
				{
					if (RegisteredSettingsNames.Contains(SettingsName))
						continue;

					SettingsRegistrationMap[SettingsName].Register(SettingsModule);
					RegisteredSettingsNames.Add(SettingsName);
				}
			};

			UpdatePlatformSettings();

			IProjectManager& ProjectManager = IProjectManager::Get();
			ProjectManager.OnTargetPlatformsForCurrentProjectChanged().AddLambda(UpdatePlatformSettings);
		}
	}

	void UnregisterSettings()
	{
		if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			auto SettingsRegistrationMap = GetWwisePlatformNameToSettingsRegistrationMap();
			for (const auto& SettingsName : RegisteredSettingsNames)
			{
				SettingsRegistrationMap[SettingsName].Unregister(SettingsModule);
			}
			RegisteredSettingsNames.Empty();
		}
	}

	/** Ak-specific viewport actions */
	FUIAction ToggleVizRoomsPortalsAction; 
	FUIAction ToggleReverbInfoAction;

	TArray<TSharedPtr<FAssetTypeActions_Base>> AkAssetTypeActionsArray;
	TSharedPtr<FExtender> MainMenuExtender;
	FLevelEditorModule::FLevelEditorMenuExtender LevelViewportToolbarBuildMenuExtenderAk;
	FDelegateHandle LevelViewportToolbarBuildMenuExtenderAkHandle;
	FDelegateHandle OnAssetRegistryFilesLoadedHandle;
	FDelegateHandle RTPCTrackEditorHandle;
	FDelegateHandle EventTrackEditorHandle;

	/** Allow to create an AkComponent when Drag & Drop of an AkEvent */
	TSharedPtr<IComponentAssetBroker> AkEventBroker;

	TWeakPtr<SWwisePickerGroup> WeakPickerGroup;

	FDoEditorBuildDelegate buildDelegate;
	AkAssetMigrationManager AssetMigrationManager;
	GeneratedSoundBanksDirectoryWatcher SoundBanksDirectoryWatcher;
};

IMPLEMENT_MODULE(FAudiokineticToolsModule, AudiokineticTools);

#undef LOCTEXT_NAMESPACE
