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

#include "AkGroupValue.h"
#include "Wwise/WwiseResourceLoader.h"

#if WITH_EDITOR
void UAkGroupValue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	LoadGroupValue(true);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITORONLY_DATA
void UAkGroupValue::MigrateIds()
{
	Super::MigrateIds();
	this->GroupValueInfo.GroupShortId = GroupShortID_DEPRECATED;
}
#endif

void UAkGroupValue::PostLoad()
{
	Super::PostLoad();
	LoadGroupValue(false);
}

void UAkGroupValue::UnloadGroupValue()
{
	if (LoadedGroupValue)
	{
		auto* ResourceLoader = UWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		ResourceLoader->UnloadGroupValue(LoadedGroupValue);
		LoadedGroupValue=nullptr;
	}
}

void UAkGroupValue::BeginDestroy()
{
	Super::BeginDestroy();
	UnloadGroupValue();
}


