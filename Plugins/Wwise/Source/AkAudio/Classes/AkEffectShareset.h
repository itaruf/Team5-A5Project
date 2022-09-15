/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.the

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "AkAudioType.h"
#include "Wwise/CookedData/WwiseLocalizedSharesetCookedData.h"
#include "Wwise/Loaded/WwiseLoadedShareset.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseAssetInfo.h"
#endif
#include "AkEffectShareset.generated.h"

UCLASS(BlueprintType)
class AKAUDIO_API UAkEffectShareSet : public UAkAudioType
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient, VisibleAnywhere, Category = "AkEffectShareSet")
	FWwiseLocalizedSharesetCookedData SharesetCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkEffectShareSet")
	FWwiseAssetInfo SharesetInfo;
#endif

public:
	void Serialize(FArchive& Ar) override;
	void LoadEffectShareset(bool bReload);
	void UnloadEffectShareset();
	void BeginDestroy() override;

	virtual void LoadData()   override {LoadEffectShareset(false);}
	virtual void ReloadData() override {LoadEffectShareset(true); }
	virtual void UnloadData() override {UnloadEffectShareset();}
	virtual AkUInt32 GetShortID() override {return SharesetCookedData.SharesetId;}

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &SharesetInfo;};
	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
#endif

private :
	FWwiseLoadedSharesetListNode* LoadedShareset;
};
