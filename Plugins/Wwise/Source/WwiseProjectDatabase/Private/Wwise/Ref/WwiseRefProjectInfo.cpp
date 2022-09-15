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

#include "Wwise/Ref/WwiseRefProjectInfo.h"

#include "Wwise/Metadata/WwiseMetadataRootFile.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefProjectInfo::NAME = TEXT("ProjectInfo");

const FWwiseMetadataProjectInfo* FWwiseRefProjectInfo::GetProjectInfo() const
{
	const auto* RootFile = GetRootFile();
	if (UNLIKELY(!RootFile))
	{
		return nullptr;
	}
	UE_CLOG(!RootFile->ProjectInfo, LogWwiseProjectDatabase, Error, TEXT("Could not get ProjectInfo"));
	return RootFile->ProjectInfo;
}
