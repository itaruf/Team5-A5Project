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
#include "Wwise/Info/WwiseGroupValueInfo.h"

#include "WwiseGroupValueInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseGroupValueInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValueInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make GroupValueInfo"))
	static
	UPARAM(DisplayName="GroupValue Info") FWwiseGroupValueInfo
	MakeStruct(
		const FGuid& AssetGuid,
		int32 GroupShortId,
		int32 AssetShortId,
		const FString& AssetName)
	{
		return FWwiseGroupValueInfo(AssetGuid, (uint32)GroupShortId, (uint32)AssetShortId, AssetName);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValueInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break GroupValueInfo"))
	static void
	BreakStruct(
		UPARAM(DisplayName="GroupValue Info") FWwiseGroupValueInfo Ref,
		FGuid& OutAssetGuid,
		int32& OutGroupShortId,
		int32& OutAssetShortId,
		FString& OutAssetName)
	{
		OutAssetGuid = Ref.AssetGuid;
		OutGroupShortId = (int32)Ref.GroupShortId;
		OutAssetShortId = (int32)Ref.AssetShortId;
		OutAssetName = Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetAssetGuid(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return Ref.AssetGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Group Short Id") int32
	GetGroupShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return (int32)Ref.GroupShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetAssetShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return (int32)Ref.AssetShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetAssetName(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetAssetGuid(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		const FGuid& AssetGuid)
	{
		auto Result = Ref;
		Result.AssetGuid = AssetGuid;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetGroupShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		int32 GroupShortId)
	{
		auto Result = Ref;
		Result.GroupShortId = (uint32)GroupShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetAssetShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		int32 AssetShortId)
	{
		auto Result = Ref;
		Result.AssetShortId = AssetShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetAssetName(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		const FString& AssetName)
	{
		auto Result = Ref;
		Result.AssetName = AssetName;
		return Result;
	}
};
