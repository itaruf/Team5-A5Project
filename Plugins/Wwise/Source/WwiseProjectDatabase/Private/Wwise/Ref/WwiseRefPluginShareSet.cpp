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

#include "Wwise/Ref/WwiseRefPluginShareSet.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefPluginShareSet::NAME = TEXT("PluginShareSet");

const FWwiseMetadataPlugin* FWwiseRefPluginShareSet::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->ShareSets;
	if (Plugins.IsValidIndex(PluginShareSetIndex))
	{
		return &Plugins[PluginShareSetIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Plugin ShareSet index #%zu"), PluginShareSetIndex);
		return nullptr;
	}
}

uint32 FWwiseRefPluginShareSet::PluginShareSetId() const
{
	const auto* PluginShareSet = GetPlugin();
	if (UNLIKELY(!PluginShareSet))
	{
		return 0;
	}
	return PluginShareSet->Id;
}

FGuid FWwiseRefPluginShareSet::PluginShareSetGuid() const
{
	const auto* PluginShareSet = GetPlugin();
	if (UNLIKELY(!PluginShareSet))
	{
		return {};
	}
	return PluginShareSet->GUID;
}

FName FWwiseRefPluginShareSet::PluginShareSetName() const
{
	const auto* PluginShareSet = GetPlugin();
	if (UNLIKELY(!PluginShareSet))
	{
		return {};
	}
	return PluginShareSet->Name;
}

FName FWwiseRefPluginShareSet::PluginShareSetObjectPath() const
{
	const auto* PluginShareSet = GetPlugin();
	if (UNLIKELY(!PluginShareSet))
	{
		return {};
	}
	return PluginShareSet->ObjectPath;
}

uint32 FWwiseRefPluginShareSet::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(PluginShareSetIndex));
	return Result;
}
