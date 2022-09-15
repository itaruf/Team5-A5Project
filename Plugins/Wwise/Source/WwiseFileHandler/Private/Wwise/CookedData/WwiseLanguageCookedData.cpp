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

#include "Wwise/CookedData/WwiseLanguageCookedData.h"

#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/Stats/FileHandler.h"
#include <inttypes.h>

const FWwiseLanguageCookedData FWwiseLanguageCookedData::Sfx(0, TEXT("SFX"), EWwiseLanguageRequirement::SFX);

FWwiseLanguageCookedData::FWwiseLanguageCookedData():
	LanguageId(0),
	LanguageName(),
	LanguageRequirement(EWwiseLanguageRequirement::IsOptional)
{}

FWwiseLanguageCookedData::FWwiseLanguageCookedData(int32 LangId, const FString& LangName, EWwiseLanguageRequirement LangRequirement):
	LanguageId(LangId),
	LanguageName(LangName),
	LanguageRequirement(LangRequirement)
{}

void FWwiseLanguageCookedData::Serialize(FArchive& Ar)
{
	UStruct* Struct = StaticStruct();
	UE_CLOG(UNLIKELY(!Struct), LogWwiseFileHandler, Fatal, TEXT("LanguageCookedData Serialize: No StaticStruct."));

	if (Ar.WantBinaryPropertySerialization())
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to binary archive %s LanguageCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), LanguageId, *LanguageName);
		Struct->SerializeBin(Ar, this);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from binary archive %s LanguageCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), LanguageId, *LanguageName);
	}
	else
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to tagged archive %s LanguageCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), LanguageId, *LanguageName);
		Struct->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(this), Struct, nullptr);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from tagged archive %s LanguageCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), LanguageId, *LanguageName);
	}
}
