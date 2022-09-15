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
#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/Loaded/WwiseLoadedInitBank.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseAssetInfo.h"
#endif

#include "AkInitBank.generated.h"


UCLASS()
class AKAUDIO_API UAkInitBank : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FWwiseInitBankCookedData InitBankCookedData;

#if WITH_EDITORONLY_DATA
	void PrepareCookedData();
#endif

	TArray<FWwiseLanguageCookedData> GetLanguages();

protected :
	FWwiseLoadedInitBankListNode* LoadedInitBank;


public:
	UAkInitBank():LoadedInitBank(nullptr){}

#if WITH_EDITORONLY_DATA
	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
	virtual FWwiseBasicInfo* GetInfoMutable() override;

#endif

	void LoadInitBank(bool bReload);
	void UnloadInitBank();

protected:
	void BeginDestroy() override;
	void Serialize(FArchive& Ar) override;
	
#if WITH_EDITORONLY_DATA
	virtual void MigrateIds() override;
#endif
};
