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

#include "SWwisePickerGroup.h"

#include "FrameWork/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Templates/SharedPointer.h"
#include "WaapiPicker/SWaapiPicker.h"
#include "Widgets/Docking/SDockTab.h"
#include "WwisePicker/SWwisePicker.h"
#include "AkAudioStyle.h"
#include "AkUEFeatures.h"

#include "IAudiokineticTools.h"
#include "AkAudioBankGenerationHelpers.h"
#include "Widgets/SUserWidget.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "SWwisePickerGroup"

//static const FName WwisePickerTabId("WwisePicker");
//static const FName WaapiPickerTabId("WaapiPicker");
static const FName WwisePickerGroupLayout("WwisePickerGroupLayout");

const FName SWwisePickerGroup::WwisePickerGroupTabName = "WwisePicker";


void SWwisePickerGroup::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	// create & initialize tab manager
	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
	TSharedRef<FWorkspaceItem> AppMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WwisePickerGroupName", "Wwise Pickers"));

	TabManager->RegisterTabSpawner(SWaapiPicker::WaapiPickerTabName, FOnSpawnTab::CreateRaw(this, &SWwisePickerGroup::CreateWaapiPickerWindow))
		.SetDisplayName(LOCTEXT("WaapiPickerTabTitle", "Waapi Picker"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkPickerTabIcon"));

	TabManager->RegisterTabSpawner(SWwisePicker::WwisePickerTabName, FOnSpawnTab::CreateRaw(this, &SWwisePickerGroup::CreateWwisePickerWindow))
		.SetDisplayName(LOCTEXT("WwisePickerTabTitle", "Wwise Picker"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkPickerTabIcon"));

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(WwisePickerGroupLayout)
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(SWaapiPicker::WaapiPickerTabName, ETabState::OpenedTab)
				->AddTab(SWwisePicker::WwisePickerTabName, ETabState::OpenedTab)
				->SetSizeCoefficient(0.75f)
				->SetForegroundTab(SWwisePicker::WwisePickerTabName)
			)
		);

	// create & initialize main menu
	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());

	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("WindowMenuLabel", "Window"),
		FText::GetEmpty(),
		FNewMenuDelegate::CreateStatic(&SWwisePickerGroup::FillWindowMenu, TabManager),
		"Window"
	);
	TSharedRef<SWidget> MenuWidget = MenuBarBuilder.MakeWidget();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MenuWidget
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()
			]
		]
	];

#if UE_5_0_OR_LATER
	TabManager->SetMenuMultiBox(MenuBarBuilder.GetMultiBox(), MenuWidget);
#else
	// Tell tab-manager about the multi-box for platforms with a global menu bar
	TabManager->SetMenuMultiBox(MenuBarBuilder.GetMultiBox());
#endif
}

void SWwisePickerGroup::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager)
{
	if (!TabManager.IsValid())
	{
		return;
	}

#if !WITH_EDITOR
	FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(MenuBuilder, WorkspaceMenu::GetMenuStructure().GetStructureRoot());
#endif //!WITH_EDITOR

	TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

TSharedRef<SDockTab> SWwisePickerGroup::CreateWaapiPickerWindow(const FSpawnTabArgs& Args)
{
	auto Tab =
		SNew(SDockTab)
		.Label(LOCTEXT("AkAudioWaapiPickerTabTitle", "Waapi Picker"))
		.TabRole(ETabRole::PanelTab)
		.ContentPadding(5)
		[
			SAssignNew(WaapiPicker, SWaapiPicker)
			.ShowGenerateSoundBanksButton(true)
		.OnDragDetected(FOnDragDetected::CreateRaw(this, &SWwisePickerGroup::HandleOnDragDetected))
		.OnImportWwiseAssetsClicked(FOnImportWwiseAssetsClicked::CreateRaw(this, &SWwisePickerGroup::HandleImportWwiseAssetsClicked))
		.OnGenerateSoundBanksClicked_Lambda([] {
		AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow();
			})
		.OnRefreshClicked_Lambda([] {
				FModuleManager::Get().GetModuleChecked<IAudiokineticTools>(FName("AudiokineticTools")).RefreshWwiseProject();
			})
		];
	Tab->SetTabIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkPickerTabIcon").GetIcon());
	return Tab;
}

TSharedRef<SDockTab> SWwisePickerGroup::CreateWwisePickerWindow(const FSpawnTabArgs& Args)
{
	auto Tab =
		SNew(SDockTab)
		.Label(LOCTEXT("AkAudioWwisePickerTabTitle", "Wwise Picker"))
		.TabRole(ETabRole::PanelTab)
		.ContentPadding(5)
		[
			SAssignNew(WwisePicker, SWwisePicker)
		];
	Tab->SetTabIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkPickerTabIcon").GetIcon());
	return Tab;
}

void SWwisePickerGroup::RequestClosePickerTabs()
{
#if UE_4_26_OR_LATER
	auto WwisePickerTab = FGlobalTabmanager::Get()->TryInvokeTab(SWwisePicker::WwisePickerTabName);
	if (WwisePickerTab.IsValid())
	{
		WwisePickerTab->RequestCloseTab();
	}
	auto WaapiPickerTab = FGlobalTabmanager::Get()->TryInvokeTab(SWaapiPicker::WaapiPickerTabName);
	if (WaapiPickerTab.IsValid())
	{
		WaapiPickerTab->RequestCloseTab();
	}
#else
	FGlobalTabmanager::Get()->InvokeTab(SWaapiPicker::WaapiPickerTabName)->RequestCloseTab();
	FGlobalTabmanager::Get()->InvokeTab(SWwisePicker::WwisePickerTabName)->RequestCloseTab();
#endif
}

void SWwisePickerGroup::UnregisterPickerTabs()
{
	if (TabManager.IsValid())
	{
		TabManager->UnregisterTabSpawner(SWaapiPicker::WaapiPickerTabName);
		TabManager->UnregisterTabSpawner(SWwisePicker::WwisePickerTabName);
	}
}

FReply SWwisePickerGroup::HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	return SWwisePicker::DoDragDetected(MouseEvent, WaapiPicker->GetSelectedItems());
}

void SWwisePickerGroup::HandleImportWwiseAssetsClicked(const FString& PackagePath)
{
	SWwisePicker::ImportWwiseAssets(WaapiPicker->GetSelectedItems(), PackagePath);
}

#undef LOCTEXT_NAMESPACE
