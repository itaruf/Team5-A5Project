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

#include "Wwise/Metadata/WwiseMetadataExternalSource.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataExternalSourceReference::FWwiseMetadataExternalSourceReference(FWwiseMetadataLoader& Loader) :
	Cookie(Loader.GetUint32(this, TEXT("Cookie")))
{
	Loader.LogParsed(TEXT("ExternalSourceReference"), Cookie);
}

FWwiseMetadataExternalSource::FWwiseMetadataExternalSource(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataExternalSourceReference(Loader),
	Name(Loader.GetString(this, TEXT("Name"))),
	ObjectPath(Loader.GetString(this, TEXT("ObjectPath"))),
	GUID(Loader.GetGuid(this, TEXT("GUID")))
{
	Loader.LogParsed(TEXT("ExternalSource"), Cookie, *Name);
}
