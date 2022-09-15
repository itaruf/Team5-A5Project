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

#include "Wwise/Ref/WwiseRefPlatformInfo.h"

#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataRootFile.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

const TCHAR* const FWwiseRefPlatformInfo::NAME = TEXT("PlatformInfo");

const FWwiseMetadataPlatformInfo* FWwiseRefPlatformInfo::GetPlatformInfo() const
{
	const auto* RootFile = GetRootFile();
	if (UNLIKELY(!RootFile))
	{
		return nullptr;
	}
	UE_CLOG(!RootFile->PlatformInfo, LogWwiseProjectDatabase, Error, TEXT("Could not get PlatformInfo"));
	return RootFile->PlatformInfo;
}
