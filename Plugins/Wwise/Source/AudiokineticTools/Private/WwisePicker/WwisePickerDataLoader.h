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
#include "WwiseItemType.h"
#include "Misc/TextFilter.h"
#include "Wwise/Metadata/WwiseMetadataBasicReference.h"
#include "Wwise/Ref/WwiseRefCollections.h"

struct FWwiseDatabaseLocalizableIdKey;
class FWwiseRefSoundBank;

using StringFilter = TTextFilter<const FString&>;
using WwiseObjectGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefSoundBank>;

struct FWwiseTreeItem;

class FWwisePickerDataLoader
{
public:
	void UpdateWwiseTree();
	TSharedPtr<FWwiseTreeItem> GetTree(TSharedPtr<StringFilter> SearchBoxFilter,
	                                   TSharedPtr<FWwiseTreeItem> CurrentTreeRootItem, EWwiseItemType::Type ItemType);
private:
	struct WwiseItemTreePath
	{
		FString HierarchyName;
		FString RootFolder;
		TArray<FString> IntermediateFolders;
		FString ItemName;
	};

	void BuildTree();
	void BuildEvents(const WwiseEventGlobalIdsMap& Events);
	void BuildBusses(const WwiseBusGlobalIdsMap& Busses);
	void BuildAuxBusses(const WwiseAuxBusGlobalIdsMap& AuxBusses);
	void BuildAcousticTextures(const WwiseAcousticTextureGlobalIdsMap& AcousticTextures);
	void BuildStateGroups(const WwiseStateGroupGlobalIdsMap& StateGroups);
	void BuildStates(const WwiseStateGlobalIdsMap& States);
	void BuildSwitchGroups(const WwiseSwitchGroupGlobalIdsMap& SwitchGroups);
	void BuildSwitches(const WwiseSwitchGlobalIdsMap& Switches);
	void BuildGameParameters(const WwiseGameParameterGlobalIdsMap& GameParameters);
	void BuildTriggers(const WwiseTriggerGlobalIdsMap& Triggers);
	void BuildEffectShareSets(const WwisePluginShareSetGlobalIdsMap& EffectShareSets);

	static bool ParseTreePath(const FString& ObjectPath, WwiseItemTreePath& OutItemPath);
	bool BuildFolderHierarchy(const FWwiseMetadataBasicReference& WwiseItem, EWwiseItemType::Type ItemType,
	                          const TSharedPtr<FWwiseTreeItem>
	                          CurrentRootFolder);

	void CopyTree(TSharedPtr<FWwiseTreeItem> SourceTreeItem, TSharedPtr<FWwiseTreeItem> DestTreeItem);

	static void FilterTree(TSharedPtr<FWwiseTreeItem> TreeItem, TSharedPtr<StringFilter> SearchBoxFilter);
	static void SortTree(TSharedPtr<FWwiseTreeItem> TreeItem);

	// Wwise objects are grouped by type
	TMap<EWwiseItemType::Type, TSharedPtr<FWwiseTreeItem>> WwiseItemTypeRoots;

	// Map of all tree items
	TMap<FGuid, TSharedPtr<FWwiseTreeItem>> AllValidTreeItemsByGuid;

	// Container paths along the Picker Tree
	TMap<FString, TSharedPtr<FWwiseTreeItem>> NodesByPath;

	FString OldFilterText;
};
