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
#include "Engine/DataTable.h"

#include "WwiseExternalSourceMediaInfo.generated.h"

//Contains the necessary info package and load an external source media
//There should be one entry for each external source media in the project 
USTRUCT(BlueprintType)
struct WWISESIMPLEEXTERNALSOURCE_API FWwiseExternalSourceMediaInfo : public FTableRowBase
{
    GENERATED_BODY()

    FWwiseExternalSourceMediaInfo(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    int32 ExternalSourceMediaInfoId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    FString MediaName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    int32 CodecID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	bool bIsStreamed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	bool bUseDeviceMemory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	int32 MemoryAlignment = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	int32 PrefetchSize = 0;

};