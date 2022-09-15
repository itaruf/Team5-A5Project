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

#include "AkMigrationWidgets.h" 
#include "AkAssetMigrationHelper.h"

#include "AkWaapiClient.h"
#include "AkSettingsPerUser.h"

#include "Widgets/Layout/SHeader.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AkUnrealEditorHelper.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace MigrationDialogUtils
{
	inline const FSlateBrush* GetExpandableAreaBorderImage(const SExpandableArea& Area)
	{
		if (Area.IsTitleHovered())
		{
			return Area.IsExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop_Hovered") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory_Hovered");
		}
		return Area.IsExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory");
	}
}

void SMigrationWidget::Construct(const FArguments& InArgs)
{
	bShowBankTransfer = InArgs._ShowBankTransfer;
	bShowMediaCleanup = InArgs._ShowDeprecatedAssetCleanup;
	bShowAssetMigration = InArgs._ShowAssetMigration;
	bShowProjectMigration = InArgs._ShowProjectMigration;
	Dialog = InArgs._Dialog;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				/// Intro title
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationWelcome",
						"Migrating to Single Source of Truth\n"
					))
					.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
				]

				/// Intro text
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationIntroText",
						"Introduced in Wwise 2022.1, Single Source of Truth is a new way of managing audio assets in your Unreal Project. Major changes are:\n"
						"- The one and only data source for the Unreal Engine Integration is now the Generated SoundBanks folder. The Wwise project is no longer parsed to gather information; everything is read from the SoundBank JSON metadata files.\n"
						"- Unreal assets no longer contain binary Wwise SoundBank and Media data.\n"
						"- Wwise-related Unreal assets can exist anywhere in your project hierarchy; there are no more rigid path requirements.\n"
						"- The new Auto-Generated SoundBanks feature in Wwise is strongly recommended, and will be enabled by default in Wwise if you were using Event-Based Packaging.\n"
						"- SoundBanks are now managed from the Wwise SoundBank manager. The event-to-SoundBank relation previously represented in Unreal assets can be imported in Wwise during the migration process.\n"
						"\n"
						"We recommend performing all necessary migration steps as soon as possible, in a single operation. Since the migration operation has an impact on Unreal assets, ensure to checkout all Wwise-related assets from Source control.\n"
					))
					.AutoWrapText(true)
				]

				/// More info hyperlink
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.0f)
					.AutoWidth()
					[
						SNew(SHyperlink)
						.Text(LOCTEXT("MigrationNotesLink", "For more information about this process, please refer to Wwise 2022.1.0 migration notes."))
						.OnNavigate_Lambda([=]{ FPlatformProcess::LaunchURL(TEXT("https://www.audiokinetic.com/library/edge/?source=UE4&id=pg_important_migration_notes_2022_1_0.html"), nullptr, nullptr); })
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SSpacer)
					]
				]

				/// Necessary steps text:
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationIntroNecessarySteps",
						"\nBelow are the necessary migration steps for your project:\n"
					))
					.AutoWrapText(true)
				]

				/// Transfer SoundBanks option
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(BankTransferWidget, SBankTransferWidget)
					.Visibility(this, &SMigrationWidget::GetBankTransferWidgetVisibility)
				]

				/// Deleting deprecated assets
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(DeprecatedAssetCleanupWidget, SDeprecatedAssetCleanupWidget)
					.Visibility(this, &SMigrationWidget::GetMediaCleanupWidgetVisibility)
					.NumDeprecatedAssets(InArgs._NumDeprecatedAssets)
				]

				/// Migrating assets
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(AssetMigrationWidget, SAssetMigrationWidget)
					.Visibility(this, &SMigrationWidget::GetAssetMigrationWidgetVisibility)
				]


				// Migrating Wwise Project options
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(ProjectMigrationWidget, SProjectMigrationWidget)
					.Visibility(this, &SMigrationWidget::GetProjectMigrationWidgetVisibility)
				]

				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GeneralInfo",
						"If you choose the cancel option or a subset of operations to perform, you can open this dialog at a later time from the Build Menu under Audiokinetic > Finish Project Migration.\n"
					))
					.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(2.0f)
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
							.ForegroundColor(FLinearColor::White)
							.OnClicked(this, &SMigrationWidget::OnContinueClicked)
							.IsEnabled(this, &SMigrationWidget::CanClickContinue)
							.ToolTipText(this, &SMigrationWidget::GetContinueToolTip)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
								.Text(FText::FromString("Continue"))
							]
						]

						+ SHorizontalBox::Slot()
						.Padding(2.0f)
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
							.ForegroundColor(FLinearColor::White)
							.OnClicked(this, &SMigrationWidget::OnCancelClicked)
							.ToolTipText(FText::FromString("Cancel"))
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
								.Text(FText::FromString("Cancel"))
							]
						]
					]
				]
			]
		]
	];
}

bool SMigrationWidget::CanClickContinue() const
{
	if (bShowBankTransfer && BankTransferWidget->SoundBankTransferCheckBox->IsChecked() && BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::NoTransfer)
	{
		return false;	
	}
	return true;
}

FText SMigrationWidget::GetContinueToolTip() const
{
	if (bShowBankTransfer && BankTransferWidget->SoundBankTransferCheckBox->IsChecked() && BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::NoTransfer)
	{
		return FText::FromString("Please choose a SoundBank transfer method first");
	}
	return  FText::FromString("Continue");
}
FReply SMigrationWidget::OnContinueClicked()
{
	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

FReply SMigrationWidget::OnCancelClicked()
{
	BankTransferWidget->BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;
	BankTransferWidget->DeleteSoundBanksCheckBox->SetIsChecked(false);
	BankTransferWidget->TransferAutoLoadCheckBox->SetIsChecked(false);
	DeprecatedAssetCleanupWidget->DeleteAssetsCheckBox->SetIsChecked(false);
	AssetMigrationWidget->MigrateAssetsCheckBox->SetIsChecked(false);
	bCancel = true;

	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

EVisibility SMigrationWidget::GetBankTransferWidgetVisibility() const
{
	return bShowBankTransfer ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetMediaCleanupWidgetVisibility() const
{
	return bShowMediaCleanup ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetAssetMigrationWidgetVisibility() const
{
	return bShowAssetMigration ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetProjectMigrationWidgetVisibility() const
{
	return bShowProjectMigration ? EVisibility::Visible : EVisibility::Collapsed;
}

void SBankTransferWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SounbankMigrationHeader", "Soundbank Migration"))
				.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("BankMigrationMessageBegin",
								"New in Wwise 2022.1 : SoundBank management has been returned to Wwise Authoring, therefore you may want to transfer your SoundBank structures back to Wwise Authoring.\n"
								"SoundBank assets are no longer used in the integration and should be deleted.\n"
							))
							.AutoWrapText(true)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Fill)
						[
							SAssignNew(ExpandableDetails, SExpandableArea)
							.InitiallyCollapsed(true)
							.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableDetails); })
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("BankMigration_WhatHappens", "Details"))
								.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
								.ShadowOffset(FVector2D(1.0f, 1.0f))
							]
							.BodyContent()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.Padding(20, 0)
									.AutoHeight()
									.HAlign(EHorizontalAlignment::HAlign_Left)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("BankMigrationMessageWhatHappens",
											"The following options will help you transfer the SoundBanks back to your Wwise project, as well as clean up these assets.\n"
											"- Transfer SoundBanks To Wwise : This will create a SoundBank in Wwise Authoring for each AkAudioBank asset in the Unreal project.\n"
											"    - Events and Buses linked to SoundBanks will be added to their corresponding banks in Wwise Authoring.\n"
											"- Transfer Auto Load property : SoundBanks' Auto Load property will be set on each of the Event and Bus assets contained in the bank.\n"
											"- Delete AkAudioBank Assets : AkAudioBank assets will be deleted after other operations have completed."

										))
										.AutoWrapText(true)
									]

									+ SVerticalBox::Slot()
									.Padding(20, 10)
									.AutoHeight()
									.HAlign(EHorizontalAlignment::HAlign_Left)
									[
										SNew(SHyperlink)
										.Text(LOCTEXT("BankMigrationMessageLink", "For more information about this process, please refer to the Integration documentation "))
										.OnNavigate_Lambda([=]() { FPlatformProcess::LaunchURL(TEXT("https://www.audiokinetic.com/library/edge/?source=UE4&id=releasenotes.html"), nullptr, nullptr); })
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(20, 0)
									.HAlign(EHorizontalAlignment::HAlign_Left)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("BankTransferDetailsText",
											"There are two methods of transferring SoundBanks : WAAPI or SoundBank Definition file\n"))
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(20, 0)
									.HAlign(EHorizontalAlignment::HAlign_Left)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("BankTransferDetailsList",
											"- If WAAPI is connected, you can choose to transfer SoundBanks and their contents directly to the Wwise project using WAAPI\n"
											"- Otherwise, you can choose to create a SoundBank Definition file, which can be manually imported into Wwise at a later time.\n"
											"   - In either case, if you also check \"Delete AkAudioBank Assets\", only banks that were successfully transferred (or written to the definition file) will be deleted.\n"
											"- If you only check \"Delete AkAudioBank Assets\", all AkAudioBank assets will be deleted and transferring banks will no longer be possible. "
										))
										.AutoWrapText(true)
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(20, 10)
									.HAlign(EHorizontalAlignment::HAlign_Left)
									[
										SNew(SHyperlink)
										.Text(LOCTEXT("BankMigrationBankDefinitionMessageLink", "SoundBank definition file documentation"))
										.OnNavigate_Lambda([=]() { FPlatformProcess::LaunchURL(TEXT("https://www.audiokinetic.com/library/edge/?source=Help&id=populating_soundbank_by_importing_definition_file"), nullptr, nullptr); })
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.FillHeight(0.10f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.FillHeight(0.10f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 2.0f)
							[
								SAssignNew(SoundBankTransferCheckBox, SCheckBox)
								.IsChecked(ECheckBoxState::Checked)
								.OnCheckStateChanged(this, &SBankTransferWidget::OnCheckedTransferBanks)
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("DoTransferMessage", " Transfer SoundBanks To Wwise"))
									.ToolTipText(LOCTEXT("TransferAkAudioBankToolTip", "Create SoundBanks in the Wwise project matching the AkAudioBank assets in Unreal.\nThey will contain the same Events and Aux Buses that were grouped into the unreal assets."))
									.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
								]
							]

							+ SHorizontalBox::Slot()
							.HAlign(EHorizontalAlignment::HAlign_Left)
							.AutoWidth()
							.Padding(8.0f, 2.0f)
							[
								SNew(SBorder)
								.BorderBackgroundColor(this, &SBankTransferWidget::GetDropDownBorderColour)
								.ColorAndOpacity(this, &SBankTransferWidget::GetDropDownColour)
								.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
								[
									SNew(SComboButton)
									.ToolTipText(LOCTEXT("ComboButtonTip", "Choose a transfer method from the drop-down menu."))
									.OnGetMenuContent(this, &SBankTransferWidget::OnGetTransferMethodMenu)
									.ContentPadding(FMargin(0))
									.Visibility(this, &SBankTransferWidget::GetTransferMethodVisibility)
									.ButtonContent()
									[
										SNew(STextBlock)
										.Text(this, &SBankTransferWidget::GetTransferMethodText)
									]
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(TransferAutoLoadCheckBox, SCheckBox)
								.IsChecked(ECheckBoxState::Checked)
								.ToolTipText(LOCTEXT("TransferAutoloadMessage", " Transfer SoundBank Auto Load property to assets"))
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TransferAutoloadMessage", "Transfer Auto Load property"))
									.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
								]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(DeleteSoundBanksCheckBox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("DeleteAkAudioBankMessage", " Delete AkAudioBank Assets "))
								.ToolTipText(LOCTEXT("DeleteAkAudioBankToolTip", "In WAAPI mode, only assets that were successfully transferred are deleted.\nIn SoundBank definition mode, the assets are deleted after the file is successfully written.\n If SoundBankTransfer is not enabled, assets will simply be deleted"))
								.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
							]
						]
					]
				]
			]
		]
	];
}

void SBankTransferWidget::SetTransferMethod(AkAssetMigration::EBankTransferMode TransferMethod)
{
	BankTransferMethod = TransferMethod;
}

void SBankTransferWidget::OnCheckedTransferBanks(ECheckBoxState NewState)
{
	if (NewState != ECheckBoxState::Checked)
	{
		BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;
	}
}

EVisibility SBankTransferWidget::GetTransferMethodVisibility() const
{
	return SoundBankTransferCheckBox->IsChecked() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SBankTransferWidget::GetTransferMethodText() const
{
	if (BankTransferMethod == AkAssetMigration::EBankTransferMode::DefinitionFile)
	{
		return 	LOCTEXT("SoundBankDefinition", "Create SoundBank Definition File");
	}
	else if (BankTransferMethod == AkAssetMigration::EBankTransferMode::WAAPI)
	{
		return LOCTEXT("WaapiTransfer", "WAAPI");
	}

	return LOCTEXT("NoTransferMethodSet", "Choose a transfer method...");
}

FSlateColor SBankTransferWidget::GetDropDownBorderColour() const
{
	if (BankTransferMethod ==  AkAssetMigration::EBankTransferMode::NoTransfer && SoundBankTransferCheckBox->IsChecked())
	{
		return FLinearColor::Red;
	}

	return FLinearColor::White;
}

FLinearColor SBankTransferWidget::GetDropDownColour() const
{
	if (BankTransferMethod ==  AkAssetMigration::EBankTransferMode::NoTransfer && SoundBankTransferCheckBox->IsChecked())
	{
		return FLinearColor::Red;
	}

	return FLinearColor::White;
}

TSharedRef<SWidget> SBankTransferWidget::OnGetTransferMethodMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("SoundBankDefinition", "Create SoundBank Definition File"),
		FText(),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SBankTransferWidget::SetTransferMethod, AkAssetMigration::EBankTransferMode::DefinitionFile)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("WaapiTransferMenuItemText","WAAPI"),
		FText(),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SBankTransferWidget::SetTransferMethod, AkAssetMigration::EBankTransferMode::WAAPI),
			FCanExecuteAction::CreateSP(this, &SBankTransferWidget::CheckWaapiConnection)
		)
	);
	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

bool SBankTransferWidget::CheckWaapiConnection() const
{

	bool bWaapiConnected = false;
	if (auto UserSettings =  GetDefault<UAkSettingsPerUser>())
	{
		if (!UserSettings->bAutoConnectToWAAPI)
		{
			LOCTEXT("WaapiTransferMenuItemText","WAAPI (Auto Connect to WAAPI disabled in user settings)");
		}
		else
		{
			FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
			bWaapiConnected = WaapiClient && WaapiClient->IsConnected();
			if (!bWaapiConnected)
			{
				LOCTEXT("WaapiTransferMenuItemText","WAAPI (WAAPI connection not established)");
			}
		}
	}

	if (bWaapiConnected)
	{
		LOCTEXT("WaapiTransferMenuItemText","WAAPI");
	}
	return bWaapiConnected;
}

void SDeprecatedAssetCleanupWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MediaCleanupHeader", "Clean up deprecated assets"))
				.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(4.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					.HAlign(EHorizontalAlignment::HAlign_Left)
					[
						SNew(STextBlock)
						.Text(FText::FormatOrdered(LOCTEXT("AssetCleanupMessageBegin",
							"AkMediaAsset, AkFolder and AkPlatformAssetData have been deprecated, all assets of this type should be removed from the project. "
							"The project currently contains {0} such assets.\n"), FText::FromString(FString::FromInt(InArgs._NumDeprecatedAssets))
						))
						.AutoWrapText(true)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					.HAlign(EHorizontalAlignment::HAlign_Fill)
					[
						SAssignNew(ExpandableDetails, SExpandableArea)
						.InitiallyCollapsed(true)
						.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableDetails); })
						.HeaderContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AssetCleanupAreaTitle", "Details"))
							.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
							.ShadowOffset(FVector2D(1.0f, 1.0f))
						]
						.BodyContent()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(20, 0)
							.HAlign(EHorizontalAlignment::HAlign_Left)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AssetCleanupDetails",
									"- All AkMediaAsset assets in the project will be deleted.\n"
									"- All AkFolder assets in the project will be deleted.\n"
									"- All AkPlatformAssetData assets (previously used for localized events) in the project will be deleted.\n"
									"- This will only delete the assets, any remaining directory structures will have to be cleaned up manually.\n"
								))
								.AutoWrapText(true)
							]
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 2.0f)
					.HAlign(EHorizontalAlignment::HAlign_Left)
					[
						SAssignNew(DeleteAssetsCheckBox, SCheckBox)
						.IsChecked(ECheckBoxState::Checked)
						.Content()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DeleteAssetMessage", " Delete deprecated assets"))
							.ToolTipText(LOCTEXT("DeleteMediaToolTip", "Delete all deprecated assets that are still in the project"))
							.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
							]
						]
					]
				]
			]
		]
	];
}

void SAssetMigrationWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UpdateAssetsHeader", "Wwise Assets are out of date"))
				.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AssetMigrationMessageBegin",
								"Wwise asset properties have changed and they no longer serialize SoundBank or media binary data.\n"
							))
							.AutoWrapText(true)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Fill)
						[
							SAssignNew(ExpandableDetails, SExpandableArea)
							.InitiallyCollapsed(true)
							.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableDetails); })
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AssetMigrateAreaTitle", "Details"))
								.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
								.ShadowOffset(FVector2D(1.0f, 1.0f))
							]
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(20, 0)
								.HAlign(EHorizontalAlignment::HAlign_Left)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AssetMigrationDetails",
										"- The only information serialized in the assets is their Wwise GUID, ShortID, and Name.\n"
										"- Checking the following option will mark all Wwise assets as dirty and save them so they can be reserialized to the disk with the correct data.\n"
										"- The Split Switch Container Media setting is removed, and will be applied to events individually.\n"
										"- WARNING : After performing this operation, it will no longer be possible to transfer the contents of AkAudioBanks to Wwise as the references will have been cleared.\n"
									))
									.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(MigrateAssetsCheckBox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("MigrateAssetsMessage", " Migrate Wwise assets"))
								.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
								.ToolTipText(LOCTEXT("MigrateAssetsTooltip", "Dirty and save all Wwise assets"))
							]
						]
					]
				]
			]
		]
	];
}

void SProjectMigrationWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ProjectChangesHeader", "Modifications to the Wwise and Unreal projects are required"))
				.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ProjectSettingsMigrationMessageBegin",
								"Some SoundBank generation settings in your Wwise project, and some Wwise Integration settings in your Unreal project, need to be modified.\n"
							))
							.AutoWrapText(true)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Fill)
						[
							SAssignNew(ExpandableDetails, SExpandableArea)
							.InitiallyCollapsed(true)
							.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableDetails); })
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ProjectMigrateDetailsHeader", "Details"))
								.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
								.ShadowOffset(FVector2D(1.0f, 1.0f))
							]
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(20, 0)
								.HAlign(EHorizontalAlignment::HAlign_Left)
								[
									SNew(STextBlock)
									.Text(FText::FormatOrdered(LOCTEXT("ProjectChanges",
										"If you were using EBP : \n"
										"  - The \"Enable Auto Defined SoundBanks\" setting will be enabled in the Wwise Project \n"
										"  - The \"Split Media Per Folder\" setting will be migrated to the \"Create Sub-Folders for Generated Files\" setting the Wwise Project \n"
										"  - The Wwise Sound Data Folder will be removed from DirectoriesToAlwaysCook \n"
										"\n"
										"If you are using the Legacy Workflow: \n"
										"  - The Wwise Sound Data Folder will be removed from DirectoriesToAlwaysStageAsUFS\n"
										"  - Generated .bnk and .wem files in {0} will be deleted\n"
										"In all cases: \n"
										"  - The \"Root Output Path\" SoundBank setting in Wwise will be imported to the \"Generated SoundBanks Folder\" integration setting in Unreal Engine.\n"
									), FText::FromString(AkUnrealEditorHelper::GetLegacySoundBankDirectory())))
									.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(AutoMigrateCheckbox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("UpdateProjectMessage", "Update project settings"))
								.Font(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
								.ToolTipText(LOCTEXT("UpdateProjectTooltip", "Update Unreal project settings"))
							]
						]
					]
				]
			]
		]
	];
}
