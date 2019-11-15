// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-21

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FPlacementItem;

class SPlacementStageObjectAssetEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlacementStageObjectAssetEntry)
    {}
		SLATE_ATTRIBUTE(FText, HighlightText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<const FPlacementItem>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<const FPlacementItem> Item;

private:
	const FSlateBrush* GetBorder() const;

	bool bIsPressed;

	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;
};