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

#include "Wwise/Ref/WwiseRefPluginLib.h"
#include "Wwise/Metadata/WwiseMetadataPluginInfo.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataPluginLib.h"

const TCHAR* const FWwiseRefPluginLib::NAME = TEXT("PluginLib");

const FWwiseMetadataPluginLib* FWwiseRefPluginLib::GetPluginLib() const
{
	const auto* PluginInfo = GetPluginInfo();
	if (UNLIKELY(!PluginInfo))
	{
		return nullptr;
	}
	const auto& PluginLibs = PluginInfo->PluginLibs;
	if (PluginLibs.IsValidIndex(PluginLibIndex))
	{
		return &PluginLibs[PluginLibIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get PluginLib index #%zu"), PluginLibIndex);
		return nullptr;
	}
}

uint32 FWwiseRefPluginLib::PluginLibId() const
{
	const auto* PluginLib = GetPluginLib();
	if (UNLIKELY(!PluginLib))
	{
		return 0;
	}
	return PluginLib->LibId;
}

FString FWwiseRefPluginLib::PluginLibName() const
{
	const auto* PluginLib = GetPluginLib();
	if (UNLIKELY(!PluginLib))
	{
		return {};
	}
	return PluginLib->LibName;
}

uint32 FWwiseRefPluginLib::Hash() const
{
	auto Result = FWwiseRefPluginInfo::Hash();
	Result = HashCombine(Result, ::GetTypeHash(PluginLibIndex));
	return Result;
}
