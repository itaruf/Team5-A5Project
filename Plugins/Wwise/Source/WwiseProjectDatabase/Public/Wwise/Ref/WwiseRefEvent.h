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

#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefEvent : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Event;
	struct FGlobalIdsMap;

	WwiseRefIndexType EventIndex;

	FWwiseRefEvent() {}
	FWwiseRefEvent(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InEventIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		EventIndex(InEventIndex)
	{}
	const FWwiseMetadataEvent* GetEvent() const;
	WwiseMediaIdsMap GetMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseMediaIdsMap GetAllMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetAllExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetAllCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginSharesetIdsMap GetPluginSharesets(const WwisePluginSharesetGlobalIdsMap& GlobalMap) const;
	WwisePluginSharesetIdsMap GetAllPluginSharesets(const WwisePluginSharesetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetAllAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	WwiseSwitchContainerArray GetSwitchContainers(const WwiseSwitchContainersByEvent& GlobalMap) const;
	WwiseEventIdsMap GetActionPostEvent(const WwiseEventGlobalIdsMap& GlobalMap) const;
	WwiseStateIdsMap GetActionSetState(const WwiseStateGlobalIdsMap& GlobalMap) const;
	WwiseSwitchIdsMap GetActionSetSwitch(const WwiseSwitchGlobalIdsMap& GlobalMap) const;
	WwiseTriggerIdsMap GetActionTrigger(const WwiseTriggerGlobalIdsMap& GlobalMap) const;
	WwiseAuxBusIdsMap GetAuxBusses(const WwiseAuxBusGlobalIdsMap& GlobalMap) const;

	uint32 EventId() const;
	FGuid EventGuid() const;
	FString EventName() const;
	FString EventObjectPath() const;
	uint32 MaxAttenuation() const;
	bool GetDuration(EWwiseMetadataEventDurationType& OutDurationType, float& OutDurationMin, float& OutDurationMax) const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefEvent& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& EventIndex == Rhs.EventIndex;
	}
	bool operator!=(const FWwiseRefEvent& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefEvent::FGlobalIdsMap
{
	WwiseEventGlobalIdsMap GlobalIdsMap;
};
