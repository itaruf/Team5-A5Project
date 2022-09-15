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

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataAcousticTexture::FWwiseMetadataAcousticTexture(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader)
{
	Loader.GetPropertyArray(this, FloatProperties);
	Loader.LogParsed(TEXT("AcousticTexture"), Id, *Name);
}

const TMap<FString, size_t> FWwiseMetadataAcousticTexture::FloatProperties = FWwiseMetadataAcousticTexture::FillFloatProperties();
const TMap<FString, size_t> FWwiseMetadataAcousticTexture::FillFloatProperties()
{
	TMap<FString, size_t> Result;
	Result.Add(TEXT("AbsorptionLow"), offsetof(FWwiseMetadataAcousticTexture, AbsorptionLow));
	Result.Add(TEXT("AbsorptionMidLow"), offsetof(FWwiseMetadataAcousticTexture, AbsorptionMidLow));
	Result.Add(TEXT("AbsorptionMidHigh"), offsetof(FWwiseMetadataAcousticTexture, AbsorptionMidHigh));
	Result.Add(TEXT("AbsorptionHigh"), offsetof(FWwiseMetadataAcousticTexture, AbsorptionHigh));
	return Result;
}
