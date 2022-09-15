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

#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataEventReference::FWwiseMetadataEventReference(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader),
	MaxAttenuation(Loader.GetUint32(this, TEXT("MaxAttenuation"), EWwiseRequiredMetadata::Optional)),
	DurationType(DurationTypeFromString(Loader.GetString(this, TEXT("DurationType")))),
	DurationMin(Loader.GetFloat(this, TEXT("DurationMin"), EWwiseRequiredMetadata::Optional)),
	DurationMax(Loader.GetFloat(this, TEXT("DurationMax"), EWwiseRequiredMetadata::Optional))
{
	IncLoadedSize(sizeof(EWwiseMetadataEventDurationType));
	Loader.LogParsed(TEXT("EventReference"), Id, *Name);
}

EWwiseMetadataEventDurationType FWwiseMetadataEventReference::DurationTypeFromString(const FString& TypeString)
{
	if (TypeString.Equals("OneShot"))
	{
		return EWwiseMetadataEventDurationType::OneShot;
	}
	else if (TypeString.Equals("Infinite"))
	{
		return EWwiseMetadataEventDurationType::Infinite;
	}
	else if (TypeString.Equals("Mixed"))
	{
		return EWwiseMetadataEventDurationType::Mixed;
	}
	else if (!TypeString.Equals("Unknown"))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataEventReference: Unknown DurationType: %s"), *TypeString);
	}
	return EWwiseMetadataEventDurationType::Unknown;
}

FWwiseMetadataEvent::FWwiseMetadataEvent(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataEventReference(Loader),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	ExternalSourceRefs(Loader.GetArray<FWwiseMetadataExternalSourceReference>(this, TEXT("ExternalSourceRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	AuxBusRefs(Loader.GetArray<FWwiseMetadataBusReference>(this, TEXT("AuxBusRefs"))),
	SwitchContainers(Loader.GetArray<FWwiseMetadataSwitchContainer>(this, TEXT("SwitchContainers"))),
	ActionPostEvent(Loader.GetArray<FWwiseMetadataActionPostEventEntry>(this, TEXT("ActionPostEvent"))),
	ActionSetState(Loader.GetArray<FWwiseMetadataActionSetStateEntry>(this, TEXT("ActionSetState"))),
	ActionSetSwitch(Loader.GetArray<FWwiseMetadataActionSetSwitchEntry>(this, TEXT("ActionSetSwitch"))),
	ActionTrigger(Loader.GetArray<FWwiseMetadataActionTriggerEntry>(this, TEXT("ActionTrigger")))
{
	Loader.LogParsed(TEXT("Event"), Id, *Name);
}

bool FWwiseMetadataEvent::IsMandatory() const
{
	return
		ActionPostEvent.Num() > 0
		|| ActionSetState.Num() > 0
		|| ActionSetSwitch.Num() > 0
		|| ActionTrigger.Num() > 0
		|| AuxBusRefs.Num() > 0
		|| ExternalSourceRefs.Num() > 0
		|| MediaRefs.Num() > 0
		|| PluginRefs && (
			PluginRefs->Custom.Num() > 0
			|| PluginRefs->Sharesets.Num() > 0
			|| PluginRefs->AudioDevices.Num() > 0)
		|| SwitchContainers.Num() == 0;
}