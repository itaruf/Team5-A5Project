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

// Defines which features of the Wwise-Unreal integration are supported in which version of UE.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "Containers/Ticker.h"

#define UE_4_26_OR_LATER ((ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26) || (ENGINE_MAJOR_VERSION >= 5))
#define UE_4_27_OR_LATER ((ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27) || (ENGINE_MAJOR_VERSION >= 5))
#define UE_5_0_OR_LATER   (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 0)

#define AK_DEPRECATED UE_DEPRECATED

/******** Bulk data defines */
#include "Serialization/BulkData.h"
// Bulk data IO request type changes depending on build settings.
#if !WITH_EDITOR
#if defined(USE_NEW_BULKDATA) && USE_NEW_BULKDATA
using BulkDataIORequest = IBulkDataIORequest;
#else
using BulkDataIORequest = FBulkDataIORequest;
#endif
#else
using BulkDataIORequest = IBulkDataIORequest;
#endif

using BulkDataRequestCompletedCallback = FBulkDataIORequestCallBack;
using ReadRequestArgumentType = IBulkDataIORequest;

// UE 5.0 typedefs
#if UE_5_0_OR_LATER
using FUnrealFloatVector = FVector3f;
using FUnrealFloatVector2D = FVector2f;
using FUnrealFloatPlane = FPlane4f;
using FTickerDelegateHandle = FTSTicker::FDelegateHandle;
using FCoreTickerType = FTSTicker;
#else
using FUnrealFloatVector = FVector;
using FUnrealFloatVector2D = FVector2D;
using FCoreTickerType = FTicker;
using FUnrealFloatPlane = FPlane;
using FTickerDelegateHandle = FDelegateHandle;
#endif


