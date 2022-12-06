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

#include "Misc/TextFilter.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/Views/STreeView.h"

using StringFilter = TTextFilter<const FString&>;

class WwisePickerBuilderVisitor;
class FWwisePickerDataLoader;

class SWwisePicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SWwisePicker )
		: _FocusSearchBoxWhenOpened(false)
		, _ShowTreeTitle(true)
		, _ShowSearchBar(true)
		, _ShowSeparator(true)
		, _AllowContextMenu(true)
		, _SelectionMode( ESelectionMode::Multi )
		{}

		/** Content displayed to the left of the search bar */
		SLATE_NAMED_SLOT( FArguments, SearchContent )

		/** If true, the search box will be focus the frame after construction */
		SLATE_ARGUMENT( bool, FocusSearchBoxWhenOpened )

		/** If true, The tree title will be displayed */
		SLATE_ARGUMENT( bool, ShowTreeTitle )

		/** If true, The tree search bar will be displayed */
		SLATE_ARGUMENT( bool, ShowSearchBar )

		/** If true, The tree search bar separator be displayed */
		SLATE_ARGUMENT( bool, ShowSeparator )

		/** If false, the context menu will be suppressed */
		SLATE_ARGUMENT( bool, AllowContextMenu )

		/** The selection mode for the tree view */
		SLATE_ARGUMENT( ESelectionMode::Type, SelectionMode )
	SLATE_END_ARGS( )

	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs);
	SWwisePicker(void);
	~SWwisePicker();

	AUDIOKINETICTOOLS_API static const FName WwisePickerTabName;

	AUDIOKINETICTOOLS_API void ForceRefresh();

	AUDIOKINETICTOOLS_API void InitialParse();
    
    virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	static FReply DoDragDetected(const FPointerEvent& MouseEvent, const TArray<TSharedPtr<FWwiseTreeItem>>& SelectedItems);

	static void ImportWwiseAssets(const TArray<TSharedPtr<FWwiseTreeItem>>& SelectedItems, const FString& PackagePath);

private:
	/** The tree view widget */
	TSharedPtr< STreeView< TSharedPtr<FWwiseTreeItem>> > TreeViewPtr;

	/** Filter for the search box */
	TSharedPtr<StringFilter> SearchBoxFilter;

	/** Root items (only events for now). */
	TArray< TSharedPtr<FWwiseTreeItem> > RootItems;

	/** Bool to prevent the selection changed callback from running */
	bool AllowTreeViewDelegates;

	/** Remember the selected items. Useful when filtering to preserve selection status. */
	TSet< FString > LastSelectedPaths;

	/** Remember the expanded items. Useful when filtering to preserve expansion status. */
	TSet< FString > LastExpandedPaths;

	//Open the Wwise Integration settings
	FReply OnOpenSettingsClicked();

	/** Ran when the Refresh button is clicked. Parses the Wwise project (using the WwiseWwuParser) and populates the window. */
	FReply OnRefreshClicked();
	
	FReply OnGenerateSoundBanksClicked();

	/** Populates the picker window only (does not parse the Wwise project) */
	void ConstructTree();

	/** Generate a row in the tree view */
	TSharedRef<ITableRow> GenerateRow( TSharedPtr<FWwiseTreeItem> TreeItem, const TSharedRef<STableViewBase>& OwnerTable );

	/** Get the children of a specific tree element */
	void GetChildrenForTree( TSharedPtr< FWwiseTreeItem > TreeItem, TArray< TSharedPtr<FWwiseTreeItem> >& OutChildren );

	/** Commands handled by this widget */
	TSharedRef<FUICommandList> CommandList;

	/** Handle Drag & Drop */
	virtual FReply OnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;

	void ExpandFirstLevel();
	void ExpandParents(TSharedPtr<FWwiseTreeItem> Item);

	FText GetProjectName() const;

	EVisibility isWarningVisible() const;
	EVisibility isWarningNotVisible() const;
	FText GetWarningText() const;

	/** Used by the search filter */
	void PopulateSearchStrings( const FString& FolderName, OUT TArray< FString >& OutSearchStrings ) const;
	void OnSearchBoxChanged( const FText& InSearchText );
	FText GetHighlightText() const;
	void FilterUpdated();
	void SetItemVisibility(TSharedPtr<FWwiseTreeItem> Item, bool IsVisible);
	void RestoreTreeExpansion(const TArray< TSharedPtr<FWwiseTreeItem> >& Items);

	/** Handler for tree view selection changes */
	void TreeSelectionChanged( TSharedPtr< FWwiseTreeItem > TreeItem, ESelectInfo::Type SelectInfo );

	/** Handler for tree view expansion changes */
	void TreeExpansionChanged( TSharedPtr< FWwiseTreeItem > TreeItem, bool bIsExpanded );

	/** Builds the command list for the context menu on Wwise Picker items. */
	void CreateWwisePickerCommands();

	/** Callback for creating a context menu for the Wwise items list. */
	TSharedPtr<SWidget> MakeWwisePickerContextMenu();

	/** Callback to import a Wwise item into the project's Contents*/
	void HandleImportWwiseItemCommandExecute() const;

	TUniquePtr<FWwisePickerDataLoader> DataLoader;
	FDelegateHandle OnDatabaseUpdateCompleteHandle;
};