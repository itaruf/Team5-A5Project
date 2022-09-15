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

#include "AkInclude.h"

class WWISEFILEHANDLER_API FWwiseIOHook
{
public:
	virtual ~FWwiseIOHook() {}

	virtual bool Init(const AkDeviceSettings& InDeviceSettings);
	virtual void Term();
	virtual AK::StreamMgr::IAkLowLevelIOHook* GetIOHook() = 0;
	virtual AK::StreamMgr::IAkFileLocationResolver* GetLocationResolver() = 0;

protected:
	FWwiseIOHook() :
		StreamingDevice(AK_INVALID_DEVICE_ID)
	{}

	AkDeviceID StreamingDevice;
};

class WWISEFILEHANDLER_API FWwiseDefaultIOHook :
	public FWwiseIOHook,
	public AK::StreamMgr::IAkIOHookDeferred,
	public AK::StreamMgr::IAkFileLocationResolver
{
public:
	AK::StreamMgr::IAkLowLevelIOHook* GetIOHook() final
	{
		return this;
	}
	AK::StreamMgr::IAkFileLocationResolver* GetLocationResolver() final
	{
		return this;
	}

protected:
	FWwiseDefaultIOHook() {}
};