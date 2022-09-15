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
#include "Wwise/Info/WwiseBasicInfo.h"

#include "WwiseBasicInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseBasicInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|BasicInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make BasicInfo"))
	static
	UPARAM(DisplayName="Basic Info") FWwiseBasicInfo
	MakeStruct(
		const FGuid& AssetGuid,
		int32 AssetShortId,
		const FString& AssetName)
	{
		return FWwiseBasicInfo(AssetGuid, (uint32)AssetShortId, AssetName);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|BasicInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break BasicInfo"))
	static void
	BreakStruct(
		UPARAM(DisplayName="Basic Info") FWwiseBasicInfo Ref,
		FGuid& OutAssetGuid,
		int32& OutAssetShortId,
		FString& OutAssetName)
	{
		OutAssetGuid = Ref.AssetGuid;
		OutAssetShortId = (int32)Ref.AssetShortId;
		OutAssetName = Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetAssetGuid(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref)
	{
		return Ref.AssetGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetAssetShortId(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref)
	{
		return (int32)Ref.AssetShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetAssetName(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref)
	{
		return Ref.AssetName;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseBasicInfo
	SetAssetGuid(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref,
		const FGuid& AssetGuid)
	{
		auto Result = Ref;
		Result.AssetGuid = AssetGuid;
		return Result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseBasicInfo
	SetAssetShortId(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref,
		int32 AssetShortId)
	{
		auto Result = Ref;
		Result.AssetShortId = AssetShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Basic Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseBasicInfo
	SetAssetName(
		UPARAM(DisplayName="Basic Info") const FWwiseBasicInfo& Ref,
		const FString& AssetName)
	{
		auto Result = Ref;
		Result.AssetName = AssetName;
		return Result;
	}
};
