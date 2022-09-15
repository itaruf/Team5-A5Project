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


#include "StaticPluginWriter.generated.h"

class FString;
USTRUCT()
struct FAkPluginInfo
{
	GENERATED_BODY()

public:
	FAkPluginInfo() = default;

	FAkPluginInfo(const FString& InName, uint32 InPluginID, const FString& InDLL)
	: Name(InName)
	, PluginID(InPluginID)
	, DLL(InDLL)
	{
	}

	UPROPERTY(VisibleAnywhere, Category = "FAkPluginInfo")
	FString Name;

	UPROPERTY(VisibleAnywhere, Category = "FAkPluginInfo")
	uint32 PluginID;

	UPROPERTY(VisibleAnywhere, Category = "FAkPluginInfo")
	FString DLL;
};


namespace StaticPluginWriter
{
	void OutputPluginInformation(const FString& Platform);
}
