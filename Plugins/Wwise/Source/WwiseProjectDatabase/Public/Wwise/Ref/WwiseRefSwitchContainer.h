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

#include "Wwise/Ref/WwiseRefEvent.h"

class WWISEPROJECTDATABASE_API FWwiseRefSwitchContainer : public FWwiseRefEvent
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::SwitchContainer;

	TArray<WwiseRefIndexType> ChildrenIndices;

	FWwiseRefSwitchContainer() {}
	FWwiseRefSwitchContainer(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InEventIndex,
		const TArray<WwiseRefIndexType>& InChildrenIndices) :
		FWwiseRefEvent(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId, InEventIndex),
		ChildrenIndices(InChildrenIndices)
	{}
	const FWwiseMetadataSwitchContainer* GetSwitchContainer() const;
	FWwiseAnyRef GetSwitchValue(const WwiseSwitchGlobalIdsMap& SwitchGlobalMap, const WwiseStateGlobalIdsMap& StateGlobalMap) const;
	WwiseMediaIdsMap GetMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginSharesetIdsMap GetPluginSharesets(const WwisePluginSharesetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	TArray<FWwiseAnyRef> GetSwitchValues(const WwiseSwitchGlobalIdsMap& SwitchGlobalMap, const WwiseStateGlobalIdsMap& StateGlobalMap) const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSwitchContainer& Rhs) const
	{
		return FWwiseRefEvent::operator ==(Rhs)
			&& ChildrenIndices == Rhs.ChildrenIndices;
	}
	bool operator!=(const FWwiseRefSwitchContainer& Rhs) const { return !operator==(Rhs); }
};
