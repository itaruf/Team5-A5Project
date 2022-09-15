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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefAcousticTexture : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::AcousticTexture;
	struct FGlobalIdsMap;

	WwiseRefIndexType AcousticTextureIndex;

	FWwiseRefAcousticTexture() {}
	FWwiseRefAcousticTexture(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InAcousticTextureIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		AcousticTextureIndex(InAcousticTextureIndex)
	{}
	const FWwiseMetadataAcousticTexture* GetAcousticTexture() const;
	
	uint32 AcousticTextureId() const;
	FGuid AcousticTextureGuid() const;
	FString AcousticTextureName() const;
	FString AcousticTextureObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefAcousticTexture& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& AcousticTextureIndex == Rhs.AcousticTextureIndex;
	}
	bool operator!=(const FWwiseRefAcousticTexture& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefAcousticTexture::FGlobalIdsMap
{
	WwiseAcousticTextureGlobalIdsMap GlobalIdsMap;

	FGlobalIdsMap() {}
	FGlobalIdsMap(const FGlobalIdsMap& Rhs) :
		GlobalIdsMap(Rhs.GlobalIdsMap)
	{}
	FGlobalIdsMap(FGlobalIdsMap&& Rhs) :
		GlobalIdsMap(MoveTemp(Rhs.GlobalIdsMap))
	{}
};
