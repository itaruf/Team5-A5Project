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

#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefCustomPlugin::NAME = TEXT("CustomPlugin");

const FWwiseMetadataPlugin* FWwiseRefCustomPlugin::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->Custom;
	if (Plugins.IsValidIndex(CustomPluginIndex))
	{
		return &Plugins[CustomPluginIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Custom Plugin index #%zu"), CustomPluginIndex);
		return nullptr;
	}
}

uint32 FWwiseRefCustomPlugin::CustomPluginId() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return 0;
	}
	return CustomPlugin->Id;
}

FGuid FWwiseRefCustomPlugin::CustomPluginGuid() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->GUID;
}

FString FWwiseRefCustomPlugin::CustomPluginName() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->Name;
}

FString FWwiseRefCustomPlugin::CustomPluginObjectPath() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->ObjectPath;
}

uint32 FWwiseRefCustomPlugin::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(CustomPluginIndex));
	return Result;
}
