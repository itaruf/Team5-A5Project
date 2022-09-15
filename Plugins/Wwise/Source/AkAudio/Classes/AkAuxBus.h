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

#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/Info/WwiseAssetInfo.h"
#include "Wwise/Loaded/WwiseLoadedAuxBus.h"

#include "AkAudioBank.h"

#include "AkAuxBus.generated.h"

class UAkAudioBank;

UCLASS(hidecategories=(Advanced, Attachment, Volume), BlueprintType)
class AKAUDIO_API UAkAuxBus : public UAkAudioType
{
	GENERATED_BODY()

public:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkAuxBus")
	FWwiseAssetInfo AuxBusInfo;
#endif

	UPROPERTY(Transient, EditAnywhere, Category = "AkAuxBus")
	FWwiseLocalizedAuxBusCookedData AuxBusCookedData;

	UPROPERTY()
	UAkAudioBank* RequiredBank_DEPRECATED = nullptr;

public:
	void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	void LoadAuxBus(bool bReload);
	void UnloadAuxBus();
	void BeginDestroy();

	virtual void LoadData()   override {LoadAuxBus(false);}
	virtual void ReloadData() override {LoadAuxBus(true); }
	virtual void UnloadData() override {UnloadAuxBus();}
	virtual AkUInt32 GetShortID() override {return AuxBusCookedData.AuxBusId;}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &AuxBusInfo;}
#endif

private:
	FWwiseLoadedAuxBusListNode* LoadedAuxBusData;

#if WITH_EDITORONLY_DATA
	virtual void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
#endif
};
