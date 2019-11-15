#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "EditorTool.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "SlateFwd.h"
#include "Widgets/SWidget.h"
#include "Widgets/SWindow.h"
#include "UnrealWidget.h"
#include "InteractionChar.h"
#include "Input/Reply.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"
#include "Engine/DataTable.h"
#include "Model/InteractionObject.h"
#include "SInteractiveObjectAssetEntry.h"

class SInteractiveEditorToolbar : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SInteractiveEditorToolbar)
	{}
	SLATE_ARGUMENT(TArray<FEditorTool*>, ToolsToRegister)
	SLATE_END_ARGS()

	SInteractiveEditorToolbar();
	~SInteractiveEditorToolbar();

	void Construct(const FArguments& InArgs);

	AInteractionChar * InteractionActor = nullptr;

	/*  创建交互物  */
	TSharedRef<SBorder> CreateInteractiveArea();
	TSharedRef<SWrapBox> CreateInterativeEditorToolsPanel();
	TSharedRef<SBorder> CreateInteractiveObjectPanel();
	TSharedRef<SWrapBox> CreateInteractiveDataToolsPanel();
	TSharedRef<SWrapBox> LogInteractivePanel();

	void LoadInteractiveData(const FString& MapFile);

	FReply AddNewInteractiveItem();
	FReply SaveInteractiveData();
	FReply ImportInteractiveData();
	FReply ExportInteractiveData();
	

	/*  创建配置  */
	TSharedRef<SBorder> CreateConfigArea();
	TSharedRef<SWrapBox> CreateConfigEditorToolsPanel();
	TSharedRef<SBorder> CreateConfigObjectPanel();
	TSharedRef<SWrapBox> CreateConfigDataToolsPanel();
	TSharedRef<SWrapBox> LogInteractionConfigPanel();

	void LoadConfigData(const FString& MapFile);

	FReply AddNewConfigItem();
	FReply AddNewSocket();
	FReply SaveConfigData();
	FReply ImportConfigData();
	FReply ExportConfigData();

private:

	/*交互物*/
	int32 SelectedInteractiveIndex;
	typedef TArray<TSharedPtr<FInteractiveObject>> InteractiveObjectItems;
	InteractiveObjectItems interactiveFilteredItems;
	TSharedPtr<SListView<TSharedPtr<FInteractiveObject>>> InteractiveObjectListView;
	FSelectInteractiveEvent TouchInteractiveItemDelegate;
	FRemoveInteractiveItemEvent OnInteractiveDataRemove;
	TSharedPtr<SInteractiveObjectAssetEntry> InteractiveObjectAssetEntry;
	FString interactiveLogStr;

	TSharedRef<ITableRow> OnGenerateInteractiveWidgetForItem(TSharedPtr<FInteractiveObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void RefreshInteractiveItemContent();
	void ClickedInteractiveItem(bool IsNew, TSharedPtr<FInteractiveObject> item);
	void RemoveInteractiveItem(TSharedPtr<FInteractiveObject> item);
	void AddInteractiveData(FInteractiveForJsonDataNodeRoot& Data, TSharedPtr<FInteractiveObject> Object);
	FText GetInteractiveLogStr() const;

	//---------------------------------------------------


	/* 配置*/
	int32 SelectedInteractionConfigIndex;
	typedef TArray<TSharedPtr<FInteractionConfigObject>> InteractionConfigObjectItems;
	InteractionConfigObjectItems interactionConfigFilteredItems;
	TSharedPtr<SListView<TSharedPtr<FInteractionConfigObject>>> InteractionConfigObjectListView;
	FSelectInteractionConfigEvent TouchInteractionConfigItemDelegate;
	FRemoveInteractionConfigItemEvent OnInteractionConfigDataRemove;
	TSharedPtr<SInteractionConfigObjectAssetEntry> InteractionConfigObjectAssetEntry;
	FString interactionConfigLogStr;

	TSharedRef<ITableRow> OnGenerateInteractionObjectWidgetForItem(TSharedPtr<FInteractionConfigObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void RefreshInteractionConfigItemContent();
	void ClickedInteractionConfigItem(bool IsNew, TSharedPtr<const FInteractionConfigObject> item);
	void RemoveInteractionConfigItem(TSharedPtr<const FInteractionConfigObject> item);
	void AddConfigData(FInteractionConfigForJsonDataNodeRoot& Data, TSharedPtr<FInteractionConfigObject> Object);
	void ClearAllChars();
	FText GetInteractionConfigLogStr() const;
};