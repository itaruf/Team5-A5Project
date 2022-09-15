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
#include "Wwise/Info/WwiseEventInfo.h"

#include "WwiseEventInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseEventInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|EventInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make EventInfo", AdvancedDisplay = "DestroyOptions, HardCodedSoundBankShortId"))
	static
	UPARAM(DisplayName="Event Info") FWwiseEventInfo
	MakeStruct(
		const FGuid& AssetGuid,
		int32 AssetShortId,
		const FString& AssetName,
		EWwiseEventSwitchContainerLoading SwitchContainerLoading,
		EWwiseEventDestroyOptions DestroyOptions,
		int32 HardCodedSoundBankShortId = 0)
	{
		return FWwiseEventInfo(AssetGuid, (uint32)AssetShortId, AssetName, SwitchContainerLoading, DestroyOptions, (uint32)HardCodedSoundBankShortId);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|EventInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break EventInfo", AdvancedDisplay = "OutDestroyOptions, OutHardCodedSoundBankShortId"))
	static void
	BreakStruct(
		UPARAM(DisplayName="Event Info") FWwiseEventInfo Ref,
		FGuid& OutAssetGuid,
		int32& OutAssetShortId,
		FString& OutAssetName,
		EWwiseEventSwitchContainerLoading& OutSwitchContainerLoading,
		EWwiseEventDestroyOptions& OutDestroyOptions,
		int32& OutHardCodedSoundBankShortId)
	{
		OutAssetGuid = Ref.AssetGuid;
		OutAssetShortId = (int32)Ref.AssetShortId;
		OutAssetName = Ref.AssetName;
		OutSwitchContainerLoading = Ref.SwitchContainerLoading;
		OutDestroyOptions = Ref.DestroyOptions;
		OutHardCodedSoundBankShortId = (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetAssetGuid(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.AssetGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetAssetShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return (int32)Ref.AssetShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetAssetName(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Switch Container Loading") EWwiseEventSwitchContainerLoading
	GetSwitchContainerLoading(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.SwitchContainerLoading;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Destroy Options") EWwiseEventDestroyOptions
	GetDestroyOptions(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.DestroyOptions;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetAssetGuid(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const FGuid& AssetGuid)
	{
		auto Result = Ref;
		Result.AssetGuid = AssetGuid;
		return Result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetAssetShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		int32 AssetShortId)
	{
		auto Result = Ref;
		Result.AssetShortId = AssetShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetAssetName(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const FString& AssetName)
	{
		auto Result = Ref;
		Result.AssetName = AssetName;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetSwitchContainerLoading(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const EWwiseEventSwitchContainerLoading& SwitchContainerLoading)
	{
		auto Result = Ref;
		Result.SwitchContainerLoading = SwitchContainerLoading;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetDestroyOptions(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const EWwiseEventDestroyOptions& DestroyOptions)
	{
		auto Result = Ref;
		Result.DestroyOptions = DestroyOptions;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		int32 HardCodedSoundBankShortId = 0)
	{
		auto Result = Ref;
		Result.HardCodedSoundBankShortId = (uint32)HardCodedSoundBankShortId;
		return Result;
	}
};
