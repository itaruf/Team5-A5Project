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

#include "Wwise/Ref/WwiseRefRootFile.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

const TCHAR* const FWwiseRefRootFile::NAME = TEXT("RootFile");

uint32 FWwiseRefRootFile::Hash() const
{
	auto Result = HashCombine(GetTypeHash(JsonFilePath), GetTypeHash(static_cast<std::underlying_type<EWwiseRefType>::type>(Type())));
	return Result;
}

bool FWwiseRefRootFile::IsValid() const
{
	return static_cast<bool>(RootFileRef);
}

const FWwiseMetadataRootFile* FWwiseRefRootFile::GetRootFile() const
{
	const auto* Result = RootFileRef.Get();
	UE_CLOG(!Result, LogWwiseProjectDatabase, Error, TEXT("Could not get Root File Ref"));
	return Result;
}
