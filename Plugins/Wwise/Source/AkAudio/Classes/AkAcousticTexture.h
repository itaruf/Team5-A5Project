/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "AkAudioType.h"
#include "Wwise/CookedData/WwiseAcousticTextureCookedData.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseAssetInfo.h"
#endif
#include "AkAcousticTexture.generated.h"

UCLASS(BlueprintType)
class AKAUDIO_API UAkAcousticTexture : public UAkAudioType
{
	GENERATED_BODY()

public :
	UPROPERTY(Transient, VisibleAnywhere, Category = "AkTexture")
	FWwiseAcousticTextureCookedData AcousticTextureCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="AkTexture")
	FLinearColor	EditColor;

	UPROPERTY(EditAnywhere, Category = "AkTexture")
	FWwiseAssetInfo AcousticTextureInfo;
#endif

public:
	void Serialize(FArchive& Ar) override;
	virtual AkUInt32 GetShortID() override {return AcousticTextureCookedData.ShortId;}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	void PostLoad() override;
	void GetAcousticTextureCookedData();
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &AcousticTextureInfo;}
#endif
};
