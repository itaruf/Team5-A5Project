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

#include "AkAcousticTexture.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkAcousticTexture::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	auto* ResourceCooker = UWwiseResourceCooker::GetForArchive(Ar);
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	FWwiseAcousticTextureCookedData CookedDataToArchive;
	if (ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(AcousticTextureInfo)))
	{
		CookedDataToArchive.Serialize(Ar);
	}
#else
	AcousticTextureCookedData.Serialize(Ar);
#endif
}

#if WITH_EDITOR
void UAkAcousticTexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GetAcousticTextureCookedData();
}
#endif 

#if WITH_EDITORONLY_DATA
void UAkAcousticTexture::PostLoad()
{
	Super::PostLoad();
	GetAcousticTextureCookedData();
}

void UAkAcousticTexture::GetAcousticTextureCookedData()
{
	if (IWwiseProjectDatabaseModule::IsInACookingCommandlet())
	{
		return;
	}
	auto* ResourceCooker = UWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}

	ResourceCooker->PrepareCookedData(AcousticTextureCookedData, GetValidatedInfo(AcousticTextureInfo));
}
#endif
