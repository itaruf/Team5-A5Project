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

#include "Wwise/Ref/WwiseRefAcousticTexture.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefAcousticTexture::NAME = TEXT("AcousticTexture");

const FWwiseMetadataAcousticTexture* FWwiseRefAcousticTexture::GetAcousticTexture() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& AcousticTextures = SoundBank->AcousticTextures;
	if (AcousticTextures.IsValidIndex(AcousticTextureIndex))
	{
		return &AcousticTextures[AcousticTextureIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Acoustic Texture index #%zu"), AcousticTextureIndex);
		return nullptr;
	}
}

uint32 FWwiseRefAcousticTexture::AcousticTextureId() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return 0;
	}
	return AcousticTexture->Id;
}

FGuid FWwiseRefAcousticTexture::AcousticTextureGuid() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->GUID;
}

FString FWwiseRefAcousticTexture::AcousticTextureName() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->Name;
}

FString FWwiseRefAcousticTexture::AcousticTextureObjectPath() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->ObjectPath;
}

uint32 FWwiseRefAcousticTexture::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, ::GetTypeHash(AcousticTextureIndex));
	return Result;
}
