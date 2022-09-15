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

#include "Wwise/Ref/WwiseRefPluginShareset.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefPluginShareset::NAME = TEXT("PluginShareset");

const FWwiseMetadataPlugin* FWwiseRefPluginShareset::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->Sharesets;
	if (Plugins.IsValidIndex(PluginSharesetIndex))
	{
		return &Plugins[PluginSharesetIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Plugin Shareset index #%zu"), PluginSharesetIndex);
		return nullptr;
	}
}

uint32 FWwiseRefPluginShareset::PluginSharesetId() const
{
	const auto* PluginShareset = GetPlugin();
	if (UNLIKELY(!PluginShareset))
	{
		return 0;
	}
	return PluginShareset->Id;
}

FGuid FWwiseRefPluginShareset::PluginSharesetGuid() const
{
	const auto* PluginShareset = GetPlugin();
	if (UNLIKELY(!PluginShareset))
	{
		return {};
	}
	return PluginShareset->GUID;
}

FString FWwiseRefPluginShareset::PluginSharesetName() const
{
	const auto* PluginShareset = GetPlugin();
	if (UNLIKELY(!PluginShareset))
	{
		return {};
	}
	return PluginShareset->Name;
}

FString FWwiseRefPluginShareset::PluginSharesetObjectPath() const
{
	const auto* PluginShareset = GetPlugin();
	if (UNLIKELY(!PluginShareset))
	{
		return {};
	}
	return PluginShareset->ObjectPath;
}

uint32 FWwiseRefPluginShareset::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(PluginSharesetIndex));
	return Result;
}
