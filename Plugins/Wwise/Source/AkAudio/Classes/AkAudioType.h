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


#include "AkInclude.h"
#include "UObject/Object.h"
#include "Wwise/Info/WwiseBasicInfo.h"
#include "Wwise/WwiseExternalSourceManager.h"

#include "AkAudioType.generated.h"

UCLASS()
class AKAUDIO_API UAkAudioType : public UObject
{
	GENERATED_BODY()

public:

UPROPERTY(VisibleAnywhere, Category = "AkAudioType|Behaviour")
bool bAutoLoad = true;

// Deprecated ID properties used in migration
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, AssetRegistrySearchable)
	FGuid WwiseGuid;

	UPROPERTY(meta=(Deprecated))
	FGuid ID_DEPRECATED;

	UPROPERTY(meta=(Deprecated))
	uint32 ShortID_DEPRECATED;
#endif
	
	UPROPERTY(EditAnywhere, Category = "AkAudioType")
	TArray<UObject*> UserData;

public:
	void Serialize(FArchive& Ar) override;
	virtual void LoadData()   {}
	virtual void ReloadData() {}
	virtual void UnloadData() {}
	void LogSerializationState(const FArchive& Ar);
	virtual AkUInt32 GetShortID() {return 0;}

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAudioType")
	int32 GetWwiseShortID() {return GetShortID();}

	template<typename T>
	T* GetUserData()
	{
		for (auto Entry : UserData)
		{
			if (Entry && Entry->GetClass()->IsChildOf(T::StaticClass()))
			{
				return Entry;
			}
		}

		return nullptr;
	}
	
#if WITH_EDITORONLY_DATA
	virtual FWwiseBasicInfo* GetInfoMutable();
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;

	template <class InfoType>
	InfoType GetValidatedInfo(const InfoType& InInfo)
	{
		InfoType TempInfo(InInfo);
		ValidateShortID(TempInfo);
		return TempInfo;
	}
#endif

protected:
#if WITH_EDITORONLY_DATA
	virtual void MigrateIds();
	void ValidateShortID(FWwiseBasicInfo& WwiseInfo) const;
#endif
};
