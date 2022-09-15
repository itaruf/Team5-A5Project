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

#include "AkAssetMigrationHelper.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SExpandableArea.h"


class SBankTransferWidget : public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS(SBankTransferWidget) {}
	SLATE_END_ARGS()

	TSharedPtr<SCheckBox> SoundBankTransferCheckBox;
	TSharedPtr<SCheckBox> TransferAutoLoadCheckBox;
	TSharedPtr<SCheckBox> DeleteSoundBanksCheckBox;
	AkAssetMigration::EBankTransferMode BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;

	void Construct(const FArguments& InArgs);
	void SetTransferMethod(AkAssetMigration::EBankTransferMode TransferMethod);
	TSharedRef<SWidget> OnGetTransferMethodMenu();

	void OnCheckedTransferBanks(ECheckBoxState NewState);
	EVisibility GetTransferMethodVisibility() const ;

	bool CheckWaapiConnection() const;
	FText GetTransferMethodText() const;
	FLinearColor GetDropDownColour() const;
	FSlateColor GetDropDownBorderColour() const;

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;

};


class SDeprecatedAssetCleanupWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDeprecatedAssetCleanupWidget) {}
		SLATE_ARGUMENT(int, NumDeprecatedAssets)
	SLATE_END_ARGS()
	TSharedPtr<SCheckBox> DeleteAssetsCheckBox;
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;

};


class SAssetMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetMigrationWidget) {}

	SLATE_END_ARGS()
	TSharedPtr<SCheckBox> MigrateAssetsCheckBox;
	void Construct(const FArguments& InArgs);

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;
};


class SProjectMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectMigrationWidget) {}
	SLATE_END_ARGS()

	TSharedPtr<SCheckBox> AutoMigrateCheckbox;
	void Construct(const FArguments& InArgs);

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;
};

class SMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMigrationWidget) {}
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Dialog)
		SLATE_ARGUMENT(bool, ShowBankTransfer)
		SLATE_ARGUMENT(bool, ShowDeprecatedAssetCleanup)
		SLATE_ARGUMENT(bool, ShowAssetMigration)
		SLATE_ARGUMENT(bool, ShowProjectMigration)
		SLATE_ARGUMENT(int, NumDeprecatedAssets)

	SLATE_END_ARGS()

	TSharedPtr<SWindow> Dialog;
	TSharedPtr<SBankTransferWidget> BankTransferWidget;
	TSharedPtr<SDeprecatedAssetCleanupWidget> DeprecatedAssetCleanupWidget;
	TSharedPtr<SAssetMigrationWidget> AssetMigrationWidget;
	TSharedPtr<SProjectMigrationWidget> ProjectMigrationWidget;

	void Construct(const FArguments& InArgs);
	FReply OnContinueClicked();
	FReply OnCancelClicked();
	EVisibility GetBankTransferWidgetVisibility() const;
	EVisibility GetMediaCleanupWidgetVisibility() const;
	EVisibility GetAssetMigrationWidgetVisibility() const;
	EVisibility GetProjectMigrationWidgetVisibility() const;

	bool CanClickContinue() const;
	FText GetContinueToolTip() const;
	bool bCancel = false;

private:
	bool bShowBankTransfer;
	bool bShowMediaCleanup;
	bool bShowAssetMigration;
	bool bShowProjectMigration;
};