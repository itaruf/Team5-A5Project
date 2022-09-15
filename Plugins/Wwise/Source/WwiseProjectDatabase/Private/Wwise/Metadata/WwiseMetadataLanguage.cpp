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

#include "Wwise/Metadata/WwiseMetadataLanguage.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataLanguageAttributes::FWwiseMetadataLanguageAttributes(FWwiseMetadataLoader& Loader) :
	Name(Loader.GetString(this, TEXT("Name"))),
	Id(Loader.GetUint32(this, TEXT("Id"))),
	GUID(Loader.GetGuid(this, TEXT("GUID"))),
	bDefault(Loader.GetBool(this, TEXT("Default"), EWwiseRequiredMetadata::Optional)),
	bUseAsStandIn(Loader.GetBool(this, TEXT("UseAsStandIn"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("LanguageAttributes"), Id, *Name);
}

FWwiseMetadataLanguage::FWwiseMetadataLanguage(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataLanguageAttributes(Loader)
{
	Loader.LogParsed(TEXT("Language"), Id, *Name);
}
