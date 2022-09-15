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


/*------------------------------------------------------------------------------------
	WwiseEventDragDropOp.h
------------------------------------------------------------------------------------*/
#pragma once

#include "Containers/Map.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "ContentBrowserDelegates.h"
#include "AssetTools/Public/AssetToolsModule.h"

class FWwiseAssetDragDropOp : public FAssetDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FWwiseEventDragDropOp, FAssetDragDropOp)

	static TSharedRef<FAssetDragDropOp> New(TMap<FAssetData, bool> InAssetData, UActorFactory* ActorFactory = nullptr);

	static TSharedRef<FAssetDragDropOp> New(TMap<FAssetData, bool> InAssetData, TArray<FString> InAssetPaths, UActorFactory* ActorFactory);

	bool OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload);
	bool OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload);

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	void SetCanDrop(const bool InCanDrop);

	void SetTooltipText();
	FText GetTooltipText() const;

	~FWwiseAssetDragDropOp();

public:
	FText CurrentHoverText;
	bool CanDrop = true;
	FAssetViewDragAndDropExtender* Extender = nullptr;

private:
	// Assets contained within the drag operation. Value is true if the asset existed prior to initiating the drop.
	TMap<FAssetData, bool> Assets;
	FAssetToolsModule* AssetToolsModule = nullptr;

};