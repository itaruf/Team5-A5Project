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

#include "Wwise/CookedData/WwiseMediaCookedData.h"

#include "Wwise/Stats/FileHandler.h"
#include <inttypes.h>

FWwiseMediaCookedData::FWwiseMediaCookedData():
	MediaId(0),
	MediaPathName(),
	PrefetchSize(0),
	MemoryAlignment(0),
	bDeviceMemory(false),
	bStreaming(false),
	DebugName()
{}

void FWwiseMediaCookedData::Serialize(FArchive& Ar)
{
	UStruct* Struct = StaticStruct();
	UE_CLOG(UNLIKELY(!Struct), LogWwiseFileHandler, Fatal, TEXT("MediaCookedData Serialize: No StaticStruct."));

	if (Ar.WantBinaryPropertySerialization())
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to binary archive %s MediaCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), MediaId, *DebugName);
		Struct->SerializeBin(Ar, this);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from binary archive %s MediaCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), MediaId, *DebugName);
	}
	else
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to tagged archive %s MediaCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), MediaId, *DebugName);
		Struct->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(this), Struct, nullptr);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from tagged archive %s MediaCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), MediaId, *DebugName);
	}
}
