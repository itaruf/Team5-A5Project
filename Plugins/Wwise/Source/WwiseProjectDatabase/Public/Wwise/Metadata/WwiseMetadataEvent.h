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

#include "Wwise/Metadata/WwiseMetadataActionEntries.h"
#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataSwitchContainer.h"

enum class EWwiseMetadataEventDurationType : uint32
{
	OneShot = 0,
	Unknown = 1,
	Infinite = 2,
	Mixed = 3
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataEventReference : public FWwiseMetadataBasicReference
{
	uint32 MaxAttenuation;
	EWwiseMetadataEventDurationType DurationType;
	float DurationMin;
	float DurationMax;

	FWwiseMetadataEventReference(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataEventDurationType DurationTypeFromString(const FString& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataEvent : public FWwiseMetadataEventReference
{
	TArray<FWwiseMetadataMediaReference> MediaRefs;
	TArray<FWwiseMetadataExternalSourceReference> ExternalSourceRefs;
	FWwiseMetadataPluginReferenceGroup* PluginRefs;
	TArray<FWwiseMetadataBusReference> AuxBusRefs;
	TArray<FWwiseMetadataSwitchContainer> SwitchContainers;
	TArray<FWwiseMetadataActionPostEventEntry> ActionPostEvent;
	TArray<FWwiseMetadataActionSetStateEntry> ActionSetState;
	TArray<FWwiseMetadataActionSetSwitchEntry> ActionSetSwitch;
	TArray<FWwiseMetadataActionTriggerEntry> ActionTrigger;

	FWwiseMetadataEvent(FWwiseMetadataLoader& Loader);

	bool IsMandatory() const;
};
