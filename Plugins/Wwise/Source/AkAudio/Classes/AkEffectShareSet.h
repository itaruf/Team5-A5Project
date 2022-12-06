/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "AkAudioType.h"
#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"
#include "Wwise/Loaded/WwiseLoadedShareSet.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseObjectInfo.h"
#endif
#include "AkEffectShareSet.generated.h"

UCLASS(BlueprintType)
class AKAUDIO_API UAkEffectShareSet : public UAkAudioType
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient, VisibleAnywhere, Category = "AkEffectShareSet")
	FWwiseLocalizedShareSetCookedData ShareSetCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkEffectShareSet")
	FWwiseObjectInfo ShareSetInfo;
#endif

public:
	void Serialize(FArchive& Ar) override;
	void BeginDestroy() override;

	virtual void LoadData()   override {LoadEffectShareSet();}
	virtual void UnloadData() override {UnloadEffectShareSet();}
	virtual AkUInt32 GetShortID() const override {return ShareSetCookedData.ShareSetId;} 

	void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual FWwiseObjectInfo* GetInfoMutable() override {return &ShareSetInfo;};
	virtual FWwiseObjectInfo GetInfo() const override{return ShareSetInfo;}

	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual void FillInfo() override;
#endif

private :
	void LoadEffectShareSet();
	void UnloadEffectShareSet();
	FWwiseLoadedShareSet LoadedShareSet;
};
