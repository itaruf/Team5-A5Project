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

#include "Wwise/Metadata/WwiseMetadataSwitchContainer.h"
#include "Wwise/Metadata/WwiseMetaDataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataSwitchContainer::FWwiseMetadataSwitchContainer(FWwiseMetadataLoader& Loader) :
	SwitchValue(Loader.GetObject<FWwiseMetadataSwitchValue>(this, TEXT("SwitchValue"))),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	ExternalSourceRefs(Loader.GetArray<FWwiseMetadataExternalSourceReference>(this, TEXT("ExternalSourceRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	Children(Loader.GetArray<FWwiseMetadataSwitchContainer>(this, TEXT("Children")))
{
	Loader.LogParsed(TEXT("SwitchContainer"));
}

TSet<FWwiseMetadataMediaReference> FWwiseMetadataSwitchContainer::GetAllMedia() const
{
	TSet<FWwiseMetadataMediaReference> Result(MediaRefs);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllMedia());
	}
	return Result;
}

TSet<FWwiseMetadataExternalSourceReference> FWwiseMetadataSwitchContainer::GetAllExternalSources() const
{
	TSet<FWwiseMetadataExternalSourceReference> Result(ExternalSourceRefs);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllExternalSources());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllCustomPlugins() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->Custom);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllCustomPlugins());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllPluginSharesets() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->Sharesets);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllPluginSharesets());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllAudioDevices() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->AudioDevices);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllAudioDevices());
	}
	return Result;
}
