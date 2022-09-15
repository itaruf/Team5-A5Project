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

#pragma once

#include "Wwise/Metadata/WwiseMetadataLoadable.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataSettings : public FWwiseMetadataLoadable
{
	bool bAutoSoundBankDefinition;
	bool bCopyLooseStreamedMediaFiles;
	bool bSubFoldersForGeneratedFiles;
	bool bRemoveUnusedGeneratedFiles;
	bool bSourceControlGeneratedFiles;
	bool bGenerateHeaderFile;
	bool bGenerateContentTxtFile;
	bool bGenerateMetadataXML;
	bool bGenerateMetadataJSON;
	bool bGenerateAllBanksMetadata;
	bool bGeneratePerBankMetadata;
	bool bUseSoundBankNames;
	bool bAllowExceedingMaxSize;
	bool bMaxAttenuationInfo;
	bool bEstimatedDurationInfo;
	bool bPrintObjectGuid;
	bool bPrintObjectPath;

	FWwiseMetadataSettings();
	FWwiseMetadataSettings(FWwiseMetadataLoader& Loader);
};
