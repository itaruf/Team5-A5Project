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

#include "Wwise/Metadata/WwiseMetadataLoader.h"

#include "Json.h"
#include <inttypes.h>

#include "Wwise/Metadata/WwiseMetadataLoadable.h"
#include "Wwise/Stats/ProjectDatabase.h"

void FWwiseMetadataLoader::Fail(const TCHAR* FieldName)
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not retrieve field %s"), FieldName);
	bResult = false;
}

void FWwiseMetadataLoader::LogParsed(const TCHAR* FieldName, const uint32 Id, const TCHAR* Name)
{
	if (bResult)
	{
		if (Id && Name)
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s [%" PRIu32 "] %s"), FieldName, Id, Name);
		}
		else if (Id)
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s [%" PRIu32 "]"), FieldName, Id);
		}
		else if (Name)
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s: %s"), FieldName, Name);
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s"), FieldName);
		}
	}
	else 
	{
		if (Id && Name)
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s [%" PRIu32 "] %s"), FieldName, Id, Name);
		}
		else if (Id)
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s [%" PRIu32 "]"), FieldName, Id);
		}
		else if (Name)
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s: %s"), FieldName, Name);
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s"), FieldName);
		}
	}
}

bool FWwiseMetadataLoader::GetBool(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("bool"), FieldName);

	bool Value = false;

	if (!JsonObject->TryGetBoolField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}

float FWwiseMetadataLoader::GetFloat(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("float"), FieldName);

	double Value{};

	if (!JsonObject->TryGetNumberField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return float(Value);
}

FGuid FWwiseMetadataLoader::GetGuid(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("guid"), FieldName);

	FGuid Value{};

	FString ValueAsString;
	if (!JsonObject->TryGetStringField(FieldName, ValueAsString))
	{
		if (Required == EWwiseRequiredMetadata::Mandatory)
		{
			Fail(*FieldName);
		}
	}
	else if (ValueAsString.Len() != 38)
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Invalid GUID %s: %s"), *FieldName, *ValueAsString);
		Fail(*FieldName);
	}
	else
	{
		if (!FGuid::ParseExact(ValueAsString, EGuidFormats::DigitsWithHyphensInBraces, Value))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not decode GUID %s: %s"), *FieldName, *ValueAsString);
			Fail(*FieldName);
		}
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}

FString FWwiseMetadataLoader::GetString(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("string"), FieldName);

	FString Value{};

	if (!JsonObject->TryGetStringField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value) + Value.GetAllocatedSize());
	return Value;
}

uint32 FWwiseMetadataLoader::GetUint32(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("uint32"), FieldName);

	uint32 Value{};

	if (!JsonObject->TryGetNumberField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}



