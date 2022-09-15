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

#include "Wwise/WwiseDatabaseIdentifiers.h"

uint32 GetTypeHash(const FWwiseDatabaseMediaIdKey& MediaId)
{
	return HashCombine(
		GetTypeHash(MediaId.MediaId),
		GetTypeHash(MediaId.SoundBankId));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableIdKey& LocalizableId)
{
	return HashCombine(
		GetTypeHash(LocalizableId.Id),
		GetTypeHash(LocalizableId.LanguageId));
}

uint32 GetTypeHash(const FWwiseDatabaseGroupValueKey& GroupId)
{
	return HashCombine(
		GetTypeHash(GroupId.GroupId),
		GetTypeHash(GroupId.Id));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableGroupValueKey& LocalizableGroupValue)
{
	return HashCombine(
		GetTypeHash(LocalizableGroupValue.GroupValue),
		GetTypeHash(LocalizableGroupValue.LanguageId));
}

uint32 GetTypeHash(const FWwiseDatabaseEventIdKey& EventId)
{
	return HashCombine(HashCombine(
		GetTypeHash(EventId.Id),
		GetTypeHash(EventId.SoundBankId)),
		GetTypeHash(EventId.LanguageId));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableGuidKey& LocalizableGuid)
{
	return HashCombine(
		GetTypeHash(LocalizableGuid.Guid),
		GetTypeHash(LocalizableGuid.LanguageId));
}
uint32 GetTypeHash(const FWwiseDatabaseLocalizableNameKey& LocalizableName)
{
	return HashCombine(
		GetTypeHash(LocalizableName.Name),
		GetTypeHash(LocalizableName.LanguageId));
}
