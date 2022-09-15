/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Wwise/Info/WwiseAssetInfo.h"

#include "WwiseAssetInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseAssetInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|AssetInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make AssetInfo", AdvancedDisplay = "HardCodedSoundBankShortId"))
	static
	UPARAM(DisplayName="Asset Info") FWwiseAssetInfo
	MakeStruct(
		const FGuid& AssetGuid,
		int32 AssetShortId,
		const FString& AssetName,
		int32 HardCodedSoundBankShortId = 0)
	{
		return FWwiseAssetInfo(AssetGuid, (uint32)AssetShortId, AssetName, (uint32)HardCodedSoundBankShortId);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|AssetInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break AssetInfo", AdvancedDisplay = "OutHardCodedSoundBankShortId"))
	static void
	BreakStruct(
		UPARAM(DisplayName="Asset Info") FWwiseAssetInfo Ref,
		FGuid& OutAssetGuid,
		int32& OutAssetShortId,
		FString& OutAssetName,
		int32& OutHardCodedSoundBankShortId)
	{
		OutAssetGuid = Ref.AssetGuid;
		OutAssetShortId = (int32)Ref.AssetShortId;
		OutAssetName = Ref.AssetName;
		OutHardCodedSoundBankShortId = (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetAssetGuid(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref)
	{
		return Ref.AssetGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetAssetShortId(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref)
	{
		return (int32)Ref.AssetShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetAssetName(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref)
	{
		return Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref)
	{
		return (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseAssetInfo
	SetAssetGuid(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref,
		const FGuid& AssetGuid)
	{
		auto Result = Ref;
		Result.AssetGuid = AssetGuid;
		return Result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseAssetInfo
	SetAssetShortId(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref,
		int32 AssetShortId)
	{
		auto Result = Ref;
		Result.AssetShortId = AssetShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseAssetInfo
	SetAssetName(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref,
		const FString& AssetName)
	{
		auto Result = Ref;
		Result.AssetName = AssetName;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Asset Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseAssetInfo
	SetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Asset Info") const FWwiseAssetInfo& Ref,
		int32 HardCodedSoundBankShortId = 0)
	{
		auto Result = Ref;
		Result.HardCodedSoundBankShortId = (uint32)HardCodedSoundBankShortId;
		return Result;
	}
};
