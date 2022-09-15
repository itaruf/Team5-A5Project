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

#include "Wwise/Metadata/WwiseMetadataForwardDeclarations.h"

#include "CoreMinimal.h"

using WwiseMetadataSharedRootFilePtr = TSharedPtr<FWwiseMetadataRootFile>;
using WwiseMetadataSharedRootFileConstPtr = TSharedPtr<const FWwiseMetadataRootFile>;
using WwiseMetadataFileMap = TMap<FString, WwiseMetadataSharedRootFilePtr>;

using WwiseMetadataStateWithGroup = TPair<const FWwiseMetadataStateGroup&, const FWwiseMetadataState&>;
using WwiseMetadataSwitchWithGroup = TPair<const FWwiseMetadataSwitchGroup&, const FWwiseMetadataSwitch&>;
