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

#include "Wwise/WwiseSharedLanguageId.h"

const FWwiseSharedLanguageId FWwiseSharedLanguageId::Sfx(FWwiseLanguageId::Sfx, EWwiseLanguageRequirement::SFX);

FWwiseSharedLanguageId::FWwiseSharedLanguageId():
	Language(new FWwiseLanguageId),
	LanguageRequirement(EWwiseLanguageRequirement::IsOptional)
{
}

FWwiseSharedLanguageId::FWwiseSharedLanguageId(int32 InLanguageId, const FString& InLanguageName, EWwiseLanguageRequirement InLanguageRequirement) :
	Language(new FWwiseLanguageId(InLanguageId, InLanguageName)),
	LanguageRequirement(InLanguageRequirement)
{
}

FWwiseSharedLanguageId::FWwiseSharedLanguageId(const FWwiseLanguageId& InLanguageId, EWwiseLanguageRequirement InLanguageRequirement):
	Language(new FWwiseLanguageId(InLanguageId)),
	LanguageRequirement(InLanguageRequirement)
{
}

FWwiseSharedLanguageId::~FWwiseSharedLanguageId()
{
}
