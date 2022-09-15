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

#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataSwitchGroup::FWwiseMetadataSwitchGroup(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader),
	GameParameterRef(Loader.GetObjectPtr<FWwiseMetadataGameParameterReference>(this, TEXT("GameParameterRef"))),
	Switches(Loader.GetArray<FWwiseMetadataSwitch>(this, TEXT("Switches")))
{
	Loader.LogParsed(TEXT("SwitchGroup"), Id, *Name);
}
