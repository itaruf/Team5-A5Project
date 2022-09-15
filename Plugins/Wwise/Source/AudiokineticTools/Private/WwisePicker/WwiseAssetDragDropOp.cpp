/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "WwisePicker/WwiseAssetDragDropOp.h"

#include "AkAudioType.h"
#include "AkAssetDropBlocker.h"
#include "AkSettings.h"
#include "AkUnrealHelper.h"
#include "AssetTools/Public/AssetToolsModule.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "ContentBrowserModule.h"
#include "Misc/Paths.h"
#include "UnrealEd/Public/ObjectTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"

TSharedRef<FAssetDragDropOp> FWwiseAssetDragDropOp::New(TMap<FAssetData, bool> InAssetData, UActorFactory* ActorFactory)
{
	return New(MoveTemp(InAssetData), TArray<FString>(), ActorFactory);
}

TSharedRef<FAssetDragDropOp> FWwiseAssetDragDropOp::New(TMap<FAssetData, bool> InAssetData, TArray<FString> InAssetPaths, UActorFactory* ActorFactory)
{
	TArray<FAssetData> NewAssets;
	InAssetData.GenerateKeyArray(NewAssets);
	TSharedRef<FAssetDragDropOp> ParentOperation = FAssetDragDropOp::New(NewAssets, InAssetPaths, ActorFactory);

	FWwiseAssetDragDropOp* RawPointer = new FWwiseAssetDragDropOp();
	TSharedRef<FWwiseAssetDragDropOp> Operation = MakeShareable(RawPointer);
	// ugly hack since FAssetDragDropOp data is private
	static_cast<FAssetDragDropOp*>(RawPointer)->operator=(ParentOperation.Get());

	FAssetViewDragAndDropExtender::FOnDropDelegate DropDelegate = FAssetViewDragAndDropExtender::FOnDropDelegate::CreateRaw(RawPointer, &FWwiseAssetDragDropOp::OnAssetViewDrop);
	FAssetViewDragAndDropExtender::FOnDragOverDelegate DragOverDelegate = FAssetViewDragAndDropExtender::FOnDragOverDelegate::CreateRaw(RawPointer, &FWwiseAssetDragDropOp::OnAssetViewDragOver);
	Operation->Extender = new FAssetViewDragAndDropExtender(DropDelegate, DragOverDelegate);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetViewDragAndDropExtender>& AssetViewDragAndDropExtenders = ContentBrowserModule.GetAssetViewDragAndDropExtenders();
	AssetViewDragAndDropExtenders.Add(*(Operation->Extender));

	Operation->Assets = InAssetData;
	Operation->AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	return Operation;
}

FWwiseAssetDragDropOp::~FWwiseAssetDragDropOp()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetViewDragAndDropExtender>& AssetViewDragAndDropExtenders = ContentBrowserModule.GetAssetViewDragAndDropExtenders();
	for (int32 i = 0; i < AssetViewDragAndDropExtenders.Num(); i++)
	{
		if (AssetViewDragAndDropExtenders[i].OnDropDelegate.IsBoundToObject(this))
		{
			AssetViewDragAndDropExtenders.RemoveAt(i);
		}
	}

	delete Extender;
}

bool FWwiseAssetDragDropOp::OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload)
{
	if (!Payload.DragDropOp->IsOfType<FWwiseAssetDragDropOp>())
	{
		SetCanDrop(false);
		return false;
	}

	if (CanDrop)
	{
		if (Payload.PackagePaths.Num() <= 0)
		{
			return false;
		}

		auto AssetDragDrop = static_cast<FWwiseAssetDragDropOp*>(Payload.DragDropOp.Get());
		auto PackagePath = Payload.PackagePaths[0].ToString();

		// UE5 adds "/All" to all game content folder paths, but CreateAsset doesn't like it
		PackagePath.RemoveFromStart(TEXT("/All"));

		// UE5 adds "/All/Plugins" to all plugin content folder paths, but CreateAsset doesn't like it
		PackagePath.RemoveFromStart(TEXT("/Plugins"));

		for (TPair<FAssetData, bool>& AssetPair: AssetDragDrop->Assets)
		{
			auto& assetData = AssetPair.Key;
			const bool bPreExisting = AssetPair.Value;
			
			if (assetData.IsValid() && PackagePath != assetData.PackagePath.ToString() 
				&& assetData.GetAsset() && !(assetData.GetAsset()->IsA<UAkAssetDropBlocker>()))
			{
				auto AkSettings = GetMutableDefault<UAkSettings>();
				const FString DefaultPath = AkSettings->DefaultAssetCreationPath;
				FString Path = "";
				if (!assetData.PackagePath.IsNone())
				{
					Path = FString(assetData.PackagePath.ToString());
				}
				Path = Path.Replace(*DefaultPath, TEXT(""));
				if(bPreExisting)
				{
					AssetToolsModule->Get().DuplicateAsset(assetData.AssetName.ToString(), PackagePath + Path, assetData.GetAsset());
				}
				else
				{
					FAssetRenameData NewAssetRenameData(assetData.GetAsset(), PackagePath + Path, assetData.AssetName.ToString());
					TArray<FAssetRenameData> assetsToRename = { NewAssetRenameData };
					AssetToolsModule->Get().RenameAssets(assetsToRename);
				}
			}
		}
	}

	return CanDrop;
}

bool FWwiseAssetDragDropOp::OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload)
{
	if (!Payload.DragDropOp->IsOfType<FWwiseAssetDragDropOp>())
	{
		SetCanDrop(false);
		return false;
	}

	SetCanDrop(true);
	return true;
}

void FWwiseAssetDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (!bDropWasHandled)
	{
		for (TPair<FAssetData, bool>& Asset: Assets)
		{
			// Asset did not previously exist, so we can safely delete it
			if (!Asset.Value)
			{
				TArray<FAssetData> AssetsToDelete = { Asset.Key };
				ObjectTools::DeleteAssets(AssetsToDelete, false);
			}
		}
	}
}

void FWwiseAssetDragDropOp::SetCanDrop(const bool InCanDrop)
{
	CanDrop = InCanDrop;
	SetTooltipText();

	if (InCanDrop)
	{
		MouseCursor = EMouseCursor::GrabHandClosed;
		SetToolTip(GetTooltipText(), NULL);
	}
	else
	{
		MouseCursor = EMouseCursor::SlashedCircle;
		SetToolTip(GetTooltipText(), FEditorStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error")));
	}
}

void FWwiseAssetDragDropOp::SetTooltipText()
{
	if (CanDrop)
	{
		auto& assets = GetAssets();
		FString Text = assets.Num() ? assets[0].AssetName.ToString() : TEXT("");

		if (assets.Num() > 1)
		{
			Text += TEXT(" ");
			Text += FString::Printf(TEXT("and %d other(s)"), assets.Num() - 1);
		}
		CurrentHoverText = FText::FromString(Text);
	}
	else
	{
		CurrentHoverText = LOCTEXT("OnDragAkEventsOverFolder_CannotDrop", "Wwise assets can only be dropped in the right folder");
	}
}

FText FWwiseAssetDragDropOp::GetTooltipText() const
{
	return CurrentHoverText;
}

#undef LOCTEXT_NAMESPACE