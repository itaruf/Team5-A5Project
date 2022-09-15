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
#include "Wwise/CookedData/WwiseGroupValueCookedData.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"
#include "Wwise/Info/WwiseGroupValueInfo.h"
#include "AkGroupValue.generated.h"

UCLASS()
class AKAUDIO_API UAkGroupValue : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, VisibleAnywhere, Category = "AkGroupValue")
	FWwiseGroupValueCookedData GroupValueCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkGroupValue")
	FWwiseGroupValueInfo GroupValueInfo;
#endif

	UPROPERTY(meta =(Deprecated,  DeprecationMessage="Use Group ID from Load Data. Used for migration from older versions."))
	uint32 GroupShortID_DEPRECATED;
	
public:
	virtual void LoadGroupValue(bool bReload){};
	void UnloadGroupValue();
	void BeginDestroy() override;

	virtual void LoadData()   override {LoadGroupValue(false);}
	virtual void ReloadData() override {LoadGroupValue(true); }
	virtual void UnloadData() override {UnloadGroupValue();}
	virtual AkUInt32 GetShortID() override {return GroupValueCookedData.Id;}
	AkUInt32 GetGroupID() {return GroupValueCookedData.GroupId;}

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &GroupValueInfo;}
#endif

protected :
	FWwiseLoadedGroupValueListNode* LoadedGroupValue;

#if WITH_EDITORONLY_DATA
	virtual void MigrateIds() override;
#endif

};
