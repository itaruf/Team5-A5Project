/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Editor.h"
#include "Wwise/WwiseProjectDatabaseImpl.h"
#include "Wwise/Stats/ProjectDatabase.h"

IMPLEMENT_MODULE(FWwiseProjectDatabaseModule, WwiseProjectDatabase)

UWwiseProjectDatabase* FWwiseProjectDatabaseModule::GetProjectDatabase()
{
	if (UNLIKELY(!GEditor))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Trying to load WwiseProjectDatabase subsystem without an editor."));
		return nullptr;
	}
	return GEditor->GetEditorSubsystem<UWwiseProjectDatabaseImpl>();
}
