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

#include "Wwise/WwiseResourceLoaderModule.h"
#include "Wwise/WwiseResourceLoader.h"

IMPLEMENT_MODULE(FWwiseResourceLoaderModule, WwiseResourceLoader)

UWwiseResourceLoader* FWwiseResourceLoaderModule::GetResourceLoader()
{
	if (UNLIKELY(!GEngine))
	{
		UE_LOG(LogWwiseResourceLoader, Warning, TEXT("Trying to load WwiseResourceLoaderImpl subsystem without an engine."));
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UWwiseResourceLoader>();
}

UWwiseResourceLoaderImpl* FWwiseResourceLoaderModule::InstantiateResourceLoaderImpl()
{
	return NewObject<UWwiseResourceLoaderImpl>();
}
