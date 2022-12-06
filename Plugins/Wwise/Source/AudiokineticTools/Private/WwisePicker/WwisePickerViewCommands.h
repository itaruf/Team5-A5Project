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
#include "Framework/Commands/Commands.h"
#include "AkAudioStyle.h"

#define LOCTEXT_NAMESPACE "WwisePickerViewCommands"


class FWwisePickerViewCommands : public TCommands<FWwisePickerViewCommands>
{

public:

	/** FWwisePickerViewCommands Constructor */
	FWwisePickerViewCommands() : TCommands<FWwisePickerViewCommands>
		(
			"WwisePickerViewCommand", // Context name for fast lookup
			NSLOCTEXT("Contexts", "WwisePickerViewCommand", "Wwise Picker Command"), // Localized context name for displaying
			NAME_None, // Parent
			FAkAudioStyle::GetStyleSetName() // Icon Style Set
			)
	{
	}

	/**
	 * Initialize the commands
	 */
	virtual void RegisterCommands() override
	{
		UI_COMMAND(RequestImportWwiseItem, "Import Selected Assets", "Imports the selected assets from the Waapi Picker.", EUserInterfaceActionType::Button, FInputChord());
	}

public:

	/** Imports the selected asset into the project's Contents */
	TSharedPtr< FUICommandInfo > RequestImportWwiseItem;
};


#undef LOCTEXT_NAMESPACE
