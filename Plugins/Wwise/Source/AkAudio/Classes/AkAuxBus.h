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

#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/Info/WwiseObjectInfo.h"
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
	FWwiseObjectInfo AuxBusInfo;
#endif

	UPROPERTY(Transient, EditAnywhere, Category = "AkAuxBus")
	FWwiseLocalizedAuxBusCookedData AuxBusCookedData;

	UPROPERTY()
	UAkAudioBank* RequiredBank_DEPRECATED = nullptr;

public:
	void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	void BeginDestroy();

	virtual void LoadData()   override {LoadAuxBus();}
	virtual void UnloadData() override {UnloadAuxBus();}
	virtual AkUInt32 GetShortID() const override {return AuxBusCookedData.AuxBusId;}

#if WITH_EDITOR
	virtual FWwiseObjectInfo* GetInfoMutable() override {return &AuxBusInfo;}
	virtual FWwiseObjectInfo GetInfo() const override{return AuxBusInfo;}
#endif

private:
	void LoadAuxBus();
	void UnloadAuxBus();
	FWwiseLoadedAuxBus LoadedAuxBus;

#if WITH_EDITORONLY_DATA
	virtual void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual void FillInfo() override;
#endif
};
