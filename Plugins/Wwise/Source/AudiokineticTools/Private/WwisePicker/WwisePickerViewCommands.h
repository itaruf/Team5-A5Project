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
