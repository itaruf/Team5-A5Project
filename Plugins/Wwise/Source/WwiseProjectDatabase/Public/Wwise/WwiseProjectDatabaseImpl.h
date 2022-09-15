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

#include "Wwise/WwiseProjectDatabase.h"

#include "WwiseProjectDatabaseImpl.generated.h"

class UWwiseResourceLoader;
class UWwiseProjectDatabase;
using FSharedWwiseDataStructure = TSharedRef<FWwiseDataStructure>;

UCLASS()
class WWISEPROJECTDATABASE_API UWwiseProjectDatabaseImpl : public UWwiseProjectDatabase
{
	GENERATED_BODY()

public:
	UWwiseProjectDatabaseImpl();
	~UWwiseProjectDatabaseImpl() override;

	UPROPERTY()
	UWwiseResourceLoader* ResourceLoaderOverride;

	void UpdateDataStructure(
		const FDirectoryPath* InUpdateGeneratedSoundBanksPath = nullptr,
		const FGuid* InBasePlatformGuid = &BasePlatformGuid) override;

	void PrepareProjectDatabaseForPlatform(UWwiseResourceLoader* InResourceLoader);
	UWwiseResourceLoader* GetResourceLoader() override;
	const UWwiseResourceLoader* GetResourceLoader() const override;

protected:
	FSharedWwiseDataStructure LockedDataStructure;

	FSharedWwiseDataStructure& GetLockedDataStructure() override { return LockedDataStructure; }
	const FSharedWwiseDataStructure& GetLockedDataStructure() const override { return LockedDataStructure; }
};