#include "WwisePickerDataLoader.h"

#include "IAudiokineticTools.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"

void FWwisePickerDataLoader::UpdateWwiseTree()
{
	// Force an update of the tree
	BuildTree();
}

TSharedPtr<FWwiseTreeItem> FWwisePickerDataLoader::GetTree(TSharedPtr<StringFilter> SearchBoxFilter,
                                                           TSharedPtr<FWwiseTreeItem> CurrentTreeRootItem,
                                                           EWwiseItemType::Type ItemType)
{
	if (!WwiseItemTypeRoots.Contains(ItemType))
	{
		return nullptr;
	}

	FString CurrentFilterText = SearchBoxFilter->GetRawFilterText().ToString();

	if (!CurrentFilterText.IsEmpty() && CurrentTreeRootItem.IsValid())
	{
		TSharedPtr<FWwiseTreeItem> FilteredTreeRootItem = MakeShared<FWwiseTreeItem>(EWwiseItemType::PickerDisplayNames[ItemType],
		    WwiseItemTypeRoots[ItemType]->FolderPath, nullptr, EWwiseItemType::Folder, FGuid(WwiseItemTypeRoots[ItemType]->ItemId));

		if (!OldFilterText.IsEmpty() && CurrentFilterText.StartsWith(OldFilterText))
		{
			CopyTree(CurrentTreeRootItem, FilteredTreeRootItem);
		}
		else
		{
			CopyTree(WwiseItemTypeRoots[ItemType], FilteredTreeRootItem);
		}

		FilterTree(FilteredTreeRootItem, SearchBoxFilter);
		OldFilterText = CurrentFilterText;
		return FilteredTreeRootItem;
	}

	SortTree(WwiseItemTypeRoots[ItemType]);

	if (WwiseItemTypeRoots[ItemType])
	{
		return WwiseItemTypeRoots[ItemType];
	}

	return nullptr;
}

void FWwisePickerDataLoader::BuildTree()
{
	UE_LOG(LogAudiokineticTools, Log, TEXT("Rebuilding tree for Wwise Picker"));

	UWwiseProjectDatabase* ProjectDatabase = UWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("BuildTree: ProjectDatabase not loaded"));
		return;
	}

	AllValidTreeItemsByGuid.Empty();
	WwiseItemTypeRoots.Empty();
	NodesByPath.Empty();

	{
		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);

		const WwiseEventGlobalIdsMap& Events = DataStructure.GetEvents();
		const WwiseBusGlobalIdsMap& Busses = DataStructure.GetBusses();
		const WwiseAuxBusGlobalIdsMap& AuxBusses = DataStructure.GetAuxBusses();
		const WwiseAcousticTextureGlobalIdsMap& AcousticTextures = DataStructure.GetAcousticTextures();
		const WwiseStateGroupGlobalIdsMap& StateGroups = DataStructure.GetStateGroups();
		const WwiseStateGlobalIdsMap& States = DataStructure.GetStates();
		const WwiseSwitchGlobalIdsMap& Switches = DataStructure.GetSwitches();
		const WwiseGameParameterGlobalIdsMap& GameParameters = DataStructure.GetGameParameters();
		const WwiseTriggerGlobalIdsMap& Triggers = DataStructure.GetTriggers();
		const WwisePluginSharesetGlobalIdsMap& EffectSharesets = DataStructure.GetPluginSharesets();

		const WwiseSwitchGroupGlobalIdsMap SwitchGroups = DataStructure.GetSwitchGroups();

		BuildEvents(Events);
		BuildBusses(Busses);
		BuildAuxBusses(AuxBusses);
		BuildAcousticTextures(AcousticTextures);
		BuildStateGroups(StateGroups);
		BuildStates(States);
		BuildSwitchGroups(SwitchGroups);
		BuildSwitches(Switches);
		BuildGameParameters(GameParameters);
		BuildTriggers(Triggers);
		BuildEffectShareSets(EffectSharesets);
	}
}

void FWwisePickerDataLoader::BuildEvents(const WwiseEventGlobalIdsMap& Events)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Event"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::Event, FolderItem);

	for (const auto& Event :Events)
	{
		const auto& WwiseItem = Event.Value.GetEvent();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Event Name: %s"), *WwiseItem->Name);

		FWwiseMetadataBasicReference EventRef = FWwiseMetadataBasicReference(WwiseItem->Id, *WwiseItem->Name, *WwiseItem->ObjectPath, FGuid(WwiseItem->GUID));
		if(!BuildFolderHierarchy(EventRef, EWwiseItemType::Event, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildBusses(const WwiseBusGlobalIdsMap& Busses)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Aux Bus"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::AuxBus, FolderItem);

	for (auto& Bus : Busses)
	{
		const auto& WwiseItem = Bus.Value.GetBus();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Bus Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::Bus, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildAuxBusses(const WwiseAuxBusGlobalIdsMap& AuxBusses)
{

	const auto& FolderItem = WwiseItemTypeRoots[EWwiseItemType::AuxBus];

	for (TTuple<FWwiseDatabaseLocalizableIdKey, FWwiseRefAuxBus> AuxBus : AuxBusses)
	{
		const auto& WwiseItem = AuxBus.Value.GetAuxBus();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Aux Bus Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::AuxBus, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildAcousticTextures(const WwiseAcousticTextureGlobalIdsMap& AcousticTextures)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("AcousticTexture"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::AcousticTexture, FolderItem);

	for (const auto& AcousticTexture :AcousticTextures)
	{
		const FWwiseMetadataAcousticTexture* WwiseItem = AcousticTexture.Value.GetAcousticTexture();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Acoustic Texture Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::AcousticTexture, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildStateGroups(const WwiseStateGroupGlobalIdsMap& StateGroups)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("State"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::State, FolderItem);

	for (const auto& StateGroup : StateGroups)
	{
		const auto& WwiseItem = StateGroup.Value.GetStateGroup();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("State Group Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::StateGroup, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildStates(const WwiseStateGlobalIdsMap& States)
{
	const auto StateGroupFolderItem = WwiseItemTypeRoots[EWwiseItemType::State];

	for (const auto& State : States)
	{
		const auto& WwiseItem = State.Value.GetState();

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::State, StateGroupFolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildSwitchGroups(const WwiseSwitchGroupGlobalIdsMap& SwitchGroups)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Switch"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::Switch, FolderItem);

	for (const auto& SwitchGroup :SwitchGroups)
	{
		const auto& WwiseItem = SwitchGroup.Value.GetSwitchGroup();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Switch Group Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::SwitchGroup, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildSwitches(const WwiseSwitchGlobalIdsMap& Switches)
{

	const auto SwitchGroupFolderItem = WwiseItemTypeRoots[EWwiseItemType::Switch];

	for (const TTuple<FWwiseDatabaseLocalizableGroupValueKey, FWwiseRefSwitch>& Switch :Switches)
	{
		const auto& WwiseItem = Switch.Value.GetSwitch();

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::Switch, SwitchGroupFolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildGameParameters(const WwiseGameParameterGlobalIdsMap& GameParameters)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Game Parameter"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::GameParameter, FolderItem);

	for (const auto& GameParameter : GameParameters)
	{
		const FWwiseMetadataGameParameter* WwiseItem = GameParameter.Value.GetGameParameter();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("GameParameter Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::GameParameter, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildTriggers(const WwiseTriggerGlobalIdsMap& Triggers)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Trigger"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::Trigger, FolderItem);

	for (const auto& Trigger :Triggers)
	{
		const auto& WwiseItem = Trigger.Value.GetTrigger();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Trigger Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::Trigger, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

void FWwisePickerDataLoader::BuildEffectShareSets(const WwisePluginSharesetGlobalIdsMap& EffectSharesets)
{
	const auto& FolderItem = MakeShared<FWwiseTreeItem>(FString("Effect Shareset"), "",  nullptr, EWwiseItemType::Folder, FGuid());
	WwiseItemTypeRoots.Add(EWwiseItemType::EffectShareSet, FolderItem);

	for (const auto& EffectShareset :EffectSharesets)
	{
		const auto& WwiseItem = EffectShareset.Value.GetPlugin();
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("Shareset Name: %s"), *WwiseItem->Name);

		if(!BuildFolderHierarchy(*WwiseItem, EWwiseItemType::EffectShareSet, FolderItem))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to place %s in the Wwise Picker"), *WwiseItem->ObjectPath);
		}
	}
}

bool FWwisePickerDataLoader::ParseTreePath(const FString& ObjectPath, WwiseItemTreePath& OutItemPath)
{
	TArray<FString> SwitchPartsArray;
	ObjectPath.ParseIntoArray(SwitchPartsArray, TEXT("\\"));

	if (SwitchPartsArray.Num() < 3)
	{
		UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to determine object hierarchy from path: %s"), *ObjectPath);
		return false;
	}

	OutItemPath.HierarchyName = SwitchPartsArray[0];
	OutItemPath.RootFolder = SwitchPartsArray[1];
	OutItemPath.ItemName = SwitchPartsArray[SwitchPartsArray.Num() - 1];

	for (int i = 2; i < SwitchPartsArray.Num() - 1; ++i)
	{
		OutItemPath.IntermediateFolders.Add(SwitchPartsArray[i]);
	}

	return true;
}

bool FWwisePickerDataLoader::BuildFolderHierarchy(
	const FWwiseMetadataBasicReference& WwiseItem, EWwiseItemType::Type
	ItemType, const TSharedPtr<FWwiseTreeItem> CurrentRootFolder)
{
		const FString ItemPath = WwiseItem.ObjectPath;
		WwiseItemTreePath TreePath;
		TSharedPtr<FWwiseTreeItem> ParentItem = CurrentRootFolder;

		if (ParseTreePath(ItemPath, TreePath))
		{
			FString CurrentPath = "\\" + TreePath.HierarchyName;
			TArray<FString> AllFolders = TArray<FString> { TreePath.RootFolder };
			AllFolders.Append(TreePath.IntermediateFolders);

			for (FString FolderName : AllFolders)
			{
				CurrentPath.Append("\\").Append(FolderName);

				TSharedPtr<FWwiseTreeItem> FolderItem;
				TSharedPtr<FWwiseTreeItem>* FolderItemPtr = NodesByPath.Find(CurrentPath);

				if (!FolderItemPtr)
				{
					FolderItem = MakeShared<FWwiseTreeItem>(FolderName, CurrentPath, ParentItem,
						EWwiseItemType::Folder, FGuid());

					NodesByPath.Add(CurrentPath, FolderItem);
					ParentItem->AddChild(FolderItem);
				}
				else
				{
					FolderItem = *FolderItemPtr;
				}

				ParentItem = FolderItem;
			}

			if (!AllValidTreeItemsByGuid.Find(WwiseItem.GUID))
			{
				const auto& NewWwiseTreeItem = MakeShared<FWwiseTreeItem>(WwiseItem.Name, WwiseItem.ObjectPath, ParentItem,
					ItemType, WwiseItem.GUID);
				ParentItem->AddChild(NewWwiseTreeItem);
				AllValidTreeItemsByGuid.Add(NewWwiseTreeItem->ItemId, NewWwiseTreeItem);

				if (ItemType == EWwiseItemType::Bus || ItemType == EWwiseItemType::SwitchGroup || ItemType ==
					EWwiseItemType::StateGroup)
				{
					NodesByPath.Add(WwiseItem.ObjectPath, NewWwiseTreeItem);
				}
			}

			return true;
		}
		return false;
}

void FWwisePickerDataLoader::CopyTree(TSharedPtr<FWwiseTreeItem> SourceTreeItem, TSharedPtr<FWwiseTreeItem> DestTreeItem)
{
	for (auto& CurrItem: SourceTreeItem->GetChildren())
	{
		TSharedPtr<FWwiseTreeItem> NewItem = MakeShared<FWwiseTreeItem>(CurrItem->DisplayName, CurrItem->FolderPath, CurrItem->Parent.Pin(), CurrItem->ItemType, CurrItem->ItemId);
		DestTreeItem->AddChild(NewItem);

		CopyTree(CurrItem, NewItem);
	}
}

void FWwisePickerDataLoader::FilterTree(TSharedPtr<FWwiseTreeItem> TreeItem, TSharedPtr<StringFilter> SearchBoxFilter)
{
	TArray<TSharedPtr<FWwiseTreeItem>> ItemsToRemove;
	for (auto& CurrItem: TreeItem->GetChildren())
	{
		FilterTree(CurrItem, SearchBoxFilter);

		if (!SearchBoxFilter->PassesFilter(CurrItem->DisplayName) && CurrItem->GetChildren().Num() == 0)
		{
			ItemsToRemove.Add(CurrItem);
		}
	}

	for (int32 i = 0; i < ItemsToRemove.Num(); i++)
	{
		TreeItem->RemoveChild(ItemsToRemove[i]);
	}
}

void FWwisePickerDataLoader::SortTree(TSharedPtr<FWwiseTreeItem> TreeItem)
{
	for (auto& CurrItem: TreeItem->GetChildren())
	{
		SortTree(CurrItem);
		CurrItem->SortChildren();
	}
}
