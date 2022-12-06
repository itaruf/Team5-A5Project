/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
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
	virtual bool CanProcessFileOp() const
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process read on a non-streaming asset"));
		return false;
	}
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
