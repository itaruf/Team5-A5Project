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
#include "Wwise/CookedData/WwiseTriggerCookedData.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseAssetInfo.h"
#endif

#include "AkTrigger.generated.h"

UCLASS(BlueprintType)
class AKAUDIO_API UAkTrigger : public UAkAudioType
{
	GENERATED_BODY()

	public :
	UPROPERTY(Transient, VisibleAnywhere, Category = "AkTexture")
	FWwiseTriggerCookedData TriggerCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkTexture")
	FWwiseAssetInfo TriggerInfo;
#endif

public:
	void Serialize(FArchive& Ar) override;

	virtual AkUInt32 GetShortID() override {return TriggerCookedData.TriggerId;}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	void PostLoad() override;
	void GetTriggerCookedData();
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &TriggerInfo;}
#endif

};
