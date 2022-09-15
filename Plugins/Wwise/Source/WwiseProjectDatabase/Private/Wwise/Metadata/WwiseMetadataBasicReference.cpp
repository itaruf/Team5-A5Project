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

#include "Wwise/Metadata/WwiseMetadataBasicReference.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataBasicReference::FWwiseMetadataBasicReference()
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Using default FWwiseMetadataBasicReference"));
}

FWwiseMetadataBasicReference::FWwiseMetadataBasicReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id"))),
	Name(Loader.GetString(this, TEXT("Name"))),
	ObjectPath(Loader.GetString(this, TEXT("ObjectPath"))),
	GUID(Loader.GetGuid(this, TEXT("GUID")))
{
	Loader.LogParsed(TEXT("BasicReference"), Id, *Name);
}
