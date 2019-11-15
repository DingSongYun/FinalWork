// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YU
// Date: 2019-02-19
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "SLevelViewport.h"
#include "SViewport.h"
#include "EditorTool.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "SlateFwd.h"
#include "Widgets/SWidget.h"
#include "Widgets/SWindow.h"
#include "UnrealWidget.h"
#include "Input/Reply.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"
#include "Engine/DataTable.h"
#include "Paper2D/Classes/PaperSprite.h"
#include "SlateBrush.h"
#include "Common/GADataTable.h"
#include "AvatarEditorTool.h"

#define REGISTER_PLACEABLE_CATEGORY(CAT)\
	static FName Get##CAT##Name() { const FName Name = #CAT; return Name; }
DECLARE_LOG_CATEGORY_EXTERN(LogAvatarEditor, Log, All);

class SPanel;
class SWrapBox;
class SLevelEditor;
enum class EMapChangeType : uint8;

struct FNpcStageObject
{
public:
	FNpcStageObject(int32 npcid)
		: Npcid(npcid)
	{
		AutoSetDisplayName();
	}

	void AutoSetDisplayName() {
		FString nameStr = UGADataTable::GetDataValueStr("NPC", Npcid, "NameZh");
		DisplayName = FText::FromString(nameStr + " ID=" + FString::FromInt(Npcid));
	}

	/** This Body's display name */
	FText DisplayName;

	int32 Npcid = 100101;

	FTransform Transform;
};

DECLARE_DELEGATE_OneParam(FSelectNpcItemEvent, TSharedPtr<const FNpcStageObject>)
DECLARE_DELEGATE(FRemoveDataEvent)
class SNpcStageObjectAssetEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNpcStageObjectAssetEntry) {}

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedPtr<const FNpcStageObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<const FNpcStageObject> Item;

	void SetSelectItemEventDelegate(FSelectNpcItemEvent SelectDelegate);
	void SetRemoveDataEventDelegate(FRemoveDataEvent RemoveDelegate);

private:
	bool bIsPressed;

	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;

	FSelectNpcItemEvent SelectItemEventDelegate;
	FRemoveDataEvent RemoveDataEventDelegate;

	FText CheckStyle(int32 npcid);

	EVisibility IsSetCurrentData(int32 npcid);

	TSharedRef<SWrapBox> CreateRemoveDataPanel();
	FReply OnRemoveDataClicked();
};

class SAvatarEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAvatarEditorToolbar)
	{}
		SLATE_ARGUMENT(TArray<FEditorTool*>, ToolsToRegister)
	SLATE_END_ARGS()

	SAvatarEditorToolbar();
	~SAvatarEditorToolbar();

	void Construct(const FArguments& InArgs);

private:

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void InitToolState();
	void OnMapChanged(UWorld* World, EMapChangeType MapChangeType);

	FDelegateHandle OnMapChangedHandler;

	bool HasRegisterTools;

	void ShowWidgetInEditor();

	const FSlateBrush* GetAvatarFrameBrush();

	const FSlateBrush* GetAvatarBrush();

	typedef TArray<TSharedPtr<FNpcStageObject>> NPCItems;
	NPCItems FilteredItems;
	TSharedPtr<SListView<TSharedPtr<FNpcStageObject>>> NpcListView;

	TSharedPtr<SNpcStageObjectAssetEntry> NpcStageObjectAssetEntry;

	TSharedRef<SBorder> CreateItemPanel();
	TSharedRef<SWrapBox> CreateEditorToolsPanel();

	FReply OnSaveDataClicked();
	FReply OnSelectCompClicked();

	void RefreshContentEvent();
	void SelectCompEvent();
	void RefreshItemContent();
	void RegisterItemObject();
	TSharedRef<ITableRow> OnGenerateWidgetForItem(TSharedPtr<FNpcStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	AAvatarEditorTool* EditorTool;
	void InitEditorTool();

	FSelectNpcItemEvent TouchItemDelegate;
	FRemoveDataEvent OnDataRemove;

	void SetupEditorChar(TSharedPtr<const FNpcStageObject> item);

	bool bNeedRefreshContent;
	void RefreshContent();
	
};


