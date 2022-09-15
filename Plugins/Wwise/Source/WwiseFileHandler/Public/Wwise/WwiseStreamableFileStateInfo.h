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

#include "Wwise/WwiseFileStateTools.h"
#include "Wwise/Stats/FileHandler.h"

#include "AkInclude.h"

class WWISEFILEHANDLER_API FWwiseStreamableFileStateInfo: protected AkFileDesc
{
public:
	static FWwiseStreamableFileStateInfo* GetFromFileDesc(AkFileDesc& InFileDesc)
	{
		return GetCustomParamAs<FWwiseStreamableFileStateInfo>(InFileDesc);
	}

	virtual ~FWwiseStreamableFileStateInfo() {}
	virtual AKRESULT ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseFileStateTools::TFileOpDoneCallback&& InFileOpDoneCallback)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process read on a non-streaming asset"));
		return AK_Fail;
	}
	virtual AKRESULT ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseFileStateTools::TFileOpDoneCallback&& InFileOpDoneCallback)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process write on a non-writable asset"));
		return AK_Fail;
	}
	virtual void CloseStreaming() {}

	template <typename RequestedType>
	static RequestedType* GetCustomParamAs(AkFileDesc& InFileDesc)
	{
		if (UNLIKELY(!InFileDesc.pCustomParam))
		{
			return nullptr;
		}
		auto* CustomParam = static_cast<FWwiseStreamableFileStateInfo*>(InFileDesc.pCustomParam);

#if defined(WITH_RTTI) || defined(_CPPRTTI) || defined(__GXX_RTTI)
		auto* Result = dynamic_cast<RequestedType*>(CustomParam);
		UE_CLOG(!Result, LogWwiseFileHandler, Fatal, TEXT("Invalid Custom Param"));
#else
		auto* Result = static_cast<RequestedType*>(CustomParam);
#endif
		return Result;
	}

	void GetFileDescCopy(AkFileDesc& OutFileDesc) const
	{
		FMemory::Memcpy(&OutFileDesc, static_cast<const AkFileDesc*>(this), sizeof(AkFileDesc));
	}

protected:
	FWwiseStreamableFileStateInfo()
	{
		FMemory::Memset(*static_cast<AkFileDesc*>(this), 0);
		uCustomParamSize = sizeof(this);
		pCustomParam = this;
	}
};
