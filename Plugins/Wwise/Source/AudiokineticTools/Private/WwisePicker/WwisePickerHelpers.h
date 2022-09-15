#pragma once

#include "Templates/SharedPointer.h"
#include "WaapiPicker/WwiseTreeItem.h"

class UObject;
class FString;

namespace WwisePickerHelpers
{
	TMap<UObject*, bool> RecurseCreateAssets(const TSharedPtr<FWwiseTreeItem>& Asset, const FString& PackagePath, const FString& CurrentPath);
	UObject* CreateAsset(const FString& AssetName, const FString& PackagePath, UClass* AssetClass);
	bool IsFolder(const TSharedPtr<FWwiseTreeItem> Item);
}
