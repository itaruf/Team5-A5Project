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

#include "CoreMinimal.h"
#include "AkInclude.h"

struct AkFileDesc;
class FWwiseFileState;

class WWISEFILEHANDLER_API IWwiseStreamingManagerHooks
{
public:
	virtual AKRESULT OpenStreaming(AkFileDesc& OutFileDesc, uint32 InShortId) = 0;
	virtual void CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState) = 0;

protected:
	virtual ~IWwiseStreamingManagerHooks() {}
};
