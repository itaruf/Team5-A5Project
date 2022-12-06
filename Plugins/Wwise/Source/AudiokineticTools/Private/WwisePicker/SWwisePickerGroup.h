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

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SCompoundWidget.h"

class FSpawnTabArgs;
class FTabManager;
class SWwisePicker;
class SWaapiPicker;

class SWwisePickerGroup : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SWwisePickerGroup) { }
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

	TSharedPtr<FTabManager> GetTabManager() const { return TabManager; }

	static const FName WwisePickerGroupTabName;

	/** Close the Waapi/Wwise pickers from the Tabmanager */
	void RequestClosePickerTabs();

	/** Unregister the Waapi/Wwise pickers from the Tabmanager */
	void UnregisterPickerTabs();

protected:

	/**
	 * Fills the Window menu with menu items.
	 *
	 * @param MenuBuilder The multi-box builder that should be filled with content for this pull-down menu.
	 * @param TabManager A Tab Manager from which to populate tab spawner menu items.
	 */
	static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager);

	TSharedRef<SDockTab> CreateWaapiPickerWindow(const FSpawnTabArgs& Args);

	TSharedRef<SDockTab> CreateWwisePickerWindow(const FSpawnTabArgs& Args);

	/** Handle drag detected for the WwisePicker*/
	FReply HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent);

	/** Handle Import Wwise Assets button clicked for the WwisePicker*/
	void HandleImportWwiseAssetsClicked(const FString& PackagePath);

private:

	/** Holds the tab for the Wwise Picker*/
	TSharedPtr<SWwisePicker> WwisePicker;

	/** Holds the tab for the Waapi Picker*/
	TSharedPtr<SWaapiPicker> WaapiPicker;

	/** Holds the tab manager that manages the front-end's tabs. */
	TSharedPtr<FTabManager> TabManager;

};
