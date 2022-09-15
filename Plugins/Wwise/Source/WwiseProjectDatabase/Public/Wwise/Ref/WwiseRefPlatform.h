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

#include "Wwise/Ref/WwiseRefPlatformInfo.h"
#include "Wwise/Ref/WwiseRefProjectInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefPlatform : public FWwiseRefPlatformInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Platform;

	// The reference does contain supplemental information, such as Path.
	FWwiseRefProjectInfo ProjectInfo;
	WwiseRefIndexType ProjectInfoPlatformReferenceIndex;

	FWwiseRefPlatform() :
		ProjectInfo(),
		ProjectInfoPlatformReferenceIndex(INDEX_NONE)
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath,
		const WwiseMetadataSharedRootFileConstPtr& InProjectInfoRootMediaRef, const FString& InProjectInfoJsonFilePath,
		WwiseRefIndexType InProjectInfoPlatformReferenceIndex) :
		FWwiseRefPlatformInfo(InRootMediaRef, InJsonFilePath),
		ProjectInfo(InProjectInfoRootMediaRef, InProjectInfoJsonFilePath),
		ProjectInfoPlatformReferenceIndex(InProjectInfoPlatformReferenceIndex)
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FString& InJsonFilePath) :
		FWwiseRefPlatformInfo(InRootMediaRef, InJsonFilePath),
		ProjectInfo(),
		ProjectInfoPlatformReferenceIndex()
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InProjectInfoRootMediaRef, const FString& InProjectInfoJsonFilePath,
		WwiseRefIndexType InProjectInfoPlatformReferenceIndex) :
		FWwiseRefPlatformInfo(),
		ProjectInfo(InProjectInfoRootMediaRef, InProjectInfoJsonFilePath),
		ProjectInfoPlatformReferenceIndex(InProjectInfoPlatformReferenceIndex)
	{}
	void Merge(FWwiseRefPlatform&& InOtherPlatform);

	const FWwiseMetadataPlatform* GetPlatform() const;
	const FWwiseMetadataPlatformReference* GetPlatformReference() const;

	FGuid PlatformGuid() const;
	FString PlatformName() const;
	FGuid BasePlatformGuid() const;
	FString BasePlatformName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
};
