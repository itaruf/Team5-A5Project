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
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/Loaded/WwiseLoadedEvent.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/Info/WwiseEventInfo.h"
#endif

#include "AkAudioEvent.generated.h"

class UAkGroupValue;
class UAkAuxBus;
class UAkAudioBank;
class UAkTrigger;
struct FStreamableHandle;


UCLASS(BlueprintType)
class AKAUDIO_API UAkAudioEvent : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MaxAttenuationRadius;

	/** Whether this event is infinite (looping) or finite (duration parameters are valid) */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		bool IsInfinite;

	/** Minimum duration */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MinimumDuration;

	/** Maximum duration */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MaximumDuration;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnyWhere, Category = "AkAudioEvent")
		FWwiseEventInfo EventInfo;
#endif

	UPROPERTY(Transient, VisibleAnywhere, Category = "AkAudioEvent")
		FWwiseLocalizedEventCookedData EventCookedData;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Used for migration"))
		UAkAudioBank* RequiredBank_DEPRECATED = nullptr;

public:
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;
	void LoadEventData(bool bReload);
	void UnloadEventData();
	void BeginDestroy() override;

	virtual void LoadData()   override {LoadEventData(false);}
	virtual void ReloadData() override {LoadEventData(true); }
	virtual void UnloadData() override {UnloadEventData();}
	virtual AkUInt32 GetShortID() override {return EventCookedData.EventId;}
	bool IsDataFullyLoaded() const;


#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	void FillMetadata(UWwiseProjectDatabase* ProjectDatabase);
	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual FWwiseBasicInfo* GetInfoMutable() override {return &EventInfo;};
#endif
	TArray<FWwiseExternalSourceCookedData> GetExternalSources() const;

private:
	FWwiseLoadedEventListNode* LoadedEventData;
};
