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

#include "Wwise/Metadata/WwiseMetadataCollections.h"
#include "Wwise/Ref/WwiseRefCollections.h"
#include "Wwise/Ref/WwiseRefType.h"

class WWISEPROJECTDATABASE_API FWwiseRefRootFile
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::RootFile;

	WwiseMetadataSharedRootFileConstPtr RootFileRef;
	FString JsonFilePath;

	FWwiseRefRootFile() {}
	FWwiseRefRootFile(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath) :
		RootFileRef(InRootMediaRef),
		JsonFilePath(InJsonFilePath)
	{}
	virtual ~FWwiseRefRootFile() {}
	virtual uint32 Hash() const;
	virtual EWwiseRefType Type() const { return TYPE; }
	bool operator==(const FWwiseRefRootFile& Rhs) const
	{
		return JsonFilePath == Rhs.JsonFilePath;
	}
	bool operator!=(const FWwiseRefRootFile& Rhs) const { return !operator==(Rhs); }

	bool IsValid() const;
	const FWwiseMetadataRootFile* GetRootFile() const;
	friend uint32 GetTypeHash(const FWwiseRefRootFile& Type)
	{
		return Type.Hash();
	}
};
