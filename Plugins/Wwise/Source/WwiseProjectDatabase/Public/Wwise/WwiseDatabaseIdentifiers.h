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

#include "CoreMinimal.h"

#include "WwiseDatabaseIdentifiers.generated.h"

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseMediaIdKey
{
	GENERATED_BODY()

	UPROPERTY() uint32 MediaId;
	UPROPERTY() uint32 SoundBankId;

	FWwiseDatabaseMediaIdKey() :
		MediaId(0),
		SoundBankId(0)
	{}
	FWwiseDatabaseMediaIdKey(uint32 InMediaId, uint32 InSoundBankId) :
		MediaId(InMediaId),
		SoundBankId(InSoundBankId)
	{}
	bool operator==(const FWwiseDatabaseMediaIdKey& Rhs) const
	{
		return MediaId == Rhs.MediaId
			&& SoundBankId == Rhs.SoundBankId;
	}
	bool operator<(const FWwiseDatabaseMediaIdKey& Rhs) const
	{
		return MediaId < Rhs.MediaId
			|| MediaId == Rhs.MediaId && SoundBankId < Rhs.SoundBankId;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableIdKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = 0;

	UPROPERTY() uint32 Id;
	UPROPERTY() uint32 LanguageId;

	FWwiseDatabaseLocalizableIdKey() :
		Id(0),
		LanguageId(0)
	{}
	FWwiseDatabaseLocalizableIdKey(uint32 InId, uint32 InLanguageId) :
		Id(InId),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableIdKey& Rhs) const
	{
		return Id == Rhs.Id
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableIdKey& Rhs) const
	{
		return Id < Rhs.Id
			|| Id == Rhs.Id && LanguageId < Rhs.LanguageId;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseGroupValueKey
{
	GENERATED_BODY()

	UPROPERTY() uint32 GroupId;
	UPROPERTY() uint32 Id;

	FWwiseDatabaseGroupValueKey() :
		GroupId(0),
		Id(0)
	{}
	FWwiseDatabaseGroupValueKey(uint32 InGroupId, uint32 InId) :
		GroupId(InGroupId),
		Id(InId)
	{}
	bool operator==(const FWwiseDatabaseGroupValueKey& Rhs) const
	{
		return GroupId == Rhs.GroupId
			&& Id == Rhs.Id;
	}
	bool operator<(const FWwiseDatabaseGroupValueKey& Rhs) const
	{
		return GroupId < Rhs.GroupId
			|| GroupId == Rhs.GroupId && Id < Rhs.Id;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableGroupValueKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = 0;

	UPROPERTY() FWwiseDatabaseGroupValueKey GroupValue;
	UPROPERTY() uint32 LanguageId;

	FWwiseDatabaseLocalizableGroupValueKey() :
		GroupValue(),
		LanguageId(0)
	{}
	FWwiseDatabaseLocalizableGroupValueKey(uint32 InGroup, uint32 InId, uint32 InLanguageId) :
		GroupValue(InGroup, InId),
		LanguageId(InLanguageId)
	{}
	FWwiseDatabaseLocalizableGroupValueKey(FWwiseDatabaseGroupValueKey InGroupValue, uint32 InLanguageId) :
		GroupValue(InGroupValue),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableGroupValueKey& Rhs) const
	{
		return GroupValue == Rhs.GroupValue
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableGroupValueKey& Rhs) const
	{
		return GroupValue < Rhs.GroupValue
			|| GroupValue == Rhs.GroupValue && LanguageId < Rhs.LanguageId;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseEventIdKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = 0;

	UPROPERTY() uint32 Id;
	UPROPERTY() uint32 LanguageId;
	UPROPERTY() uint32 SoundBankId;

	FWwiseDatabaseEventIdKey() :
		Id(0),
		LanguageId(0),
		SoundBankId(0)
	{}
	FWwiseDatabaseEventIdKey(uint32 InId, uint32 InLanguageId, uint32 InSoundBankId) :
		Id(InId),
		LanguageId(InLanguageId),
		SoundBankId(InSoundBankId)
	{}
	bool operator==(const FWwiseDatabaseEventIdKey& Rhs) const
	{
		return Id == Rhs.Id
			&& LanguageId == Rhs.LanguageId
			&& SoundBankId == Rhs.SoundBankId;
	}
	bool operator<(const FWwiseDatabaseEventIdKey& Rhs) const
	{
		return Id < Rhs.Id
			|| Id == Rhs.Id && LanguageId < Rhs.LanguageId
			|| Id == Rhs.Id && LanguageId == Rhs.LanguageId && SoundBankId < Rhs.SoundBankId;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableGuidKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE;

	UPROPERTY() FGuid Guid;
	UPROPERTY() uint32 LanguageId;		// 0 if no Language

	FWwiseDatabaseLocalizableGuidKey() :
		Guid(),
		LanguageId(0)
	{}
	FWwiseDatabaseLocalizableGuidKey(FGuid InGuid, uint32 InLanguageId) :
		Guid(InGuid),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableGuidKey& Rhs) const
	{
		return Guid == Rhs.Guid
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableGuidKey& Rhs) const
	{
		return Guid < Rhs.Guid
			|| Guid == Rhs.Guid && LanguageId < Rhs.LanguageId;
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableNameKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE;

	UPROPERTY() FString Name;
	UPROPERTY() uint32 LanguageId;		// 0 if no Language

	FWwiseDatabaseLocalizableNameKey() :
		Name(),
		LanguageId(0)
	{}
	FWwiseDatabaseLocalizableNameKey(FString InName, uint32 InLanguageId) :
		Name(InName),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableNameKey& Rhs) const
	{
		return Name == Rhs.Name
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableNameKey& Rhs) const
	{
		return Name < Rhs.Name
			|| Name == Rhs.Name && LanguageId < Rhs.LanguageId;
	}
};

uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseMediaIdKey& FileId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableIdKey& LocalizableId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseGroupValueKey& LocalizableGroupValue);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableGroupValueKey& LocalizableGroupValue);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseEventIdKey& EventId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableGuidKey& LocalizableGuid);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableNameKey& LocalizableName);
