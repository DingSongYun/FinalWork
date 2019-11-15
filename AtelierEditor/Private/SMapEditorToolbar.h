// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#pragma once

#include "EditorTool.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetData.h"
#include "Widgets/Views/STableRow.h"
#include "System/Project.h"

class SPanel;
class SButton;
class SWrapBox;
class SSplitter;
class STextBlock;
struct FAssetData;
class UActorFactory;
enum class EMapChangeType : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogAtelierMapEditor, Log, All);

#define REGISTER_PLACEABLE_CATEGORY(CAT)\
	static FName Get##CAT##Name() { const FName Name = #CAT; return Name; }

class FPlaceableCategory
{
public:
	REGISTER_PLACEABLE_CATEGORY(Basic)
	REGISTER_PLACEABLE_CATEGORY(Spawner)
	REGISTER_PLACEABLE_CATEGORY(Character)
	REGISTER_PLACEABLE_CATEGORY(Monster)
	REGISTER_PLACEABLE_CATEGORY(Building)
	REGISTER_PLACEABLE_CATEGORY(PlacementObject)
};
class PlacementTabInfo
{
public:
	PlacementTabInfo( FName Name )
		: Name(Name) {}
	FName Name;
};

struct FPlacementStageObject
{
public:
	FPlacementStageObject(UActorFactory* InFactory, const FAssetData& InAssetData)
		: Factory(InFactory)
		, AssetData(InAssetData)
	{
		AutoSetDisplayName();
	}

	void AutoSetDisplayName()
	{
		const bool bIsClass = AssetData.GetClass() == UClass::StaticClass();
		const bool bIsActor = bIsClass ? CastChecked<UClass>(AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

		if (bIsActor)
		{
			AActor* DefaultActor = CastChecked<AActor>(CastChecked<UClass>(AssetData.GetAsset())->ClassDefaultObject);
			DisplayName = FText::FromString(FName::NameToDisplayString(DefaultActor->GetClass()->GetName(), false));
		}
		else if (bIsClass)
		{
			DisplayName = FText::FromString(FName::NameToDisplayString(AssetData.AssetName.ToString(), false));
		}
		else
		{
			DisplayName = FText::FromName(AssetData.AssetName);
		}
	}

	/** The factory used to create an instance of this placeable item */
	UActorFactory* Factory;

	/** Asset data pertaining to the class */
	FAssetData AssetData;

	/** This item's display name */
	FText DisplayName;
};

class SPlacementStageObjectAssetEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlacementStageObjectAssetEntry){}

		/** Highlight this text in the text block */
		SLATE_ATTRIBUTE(FText, HighlightText)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<const FPlacementStageObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<const FPlacementStageObject> Item;

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

class SMapEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMapEditorToolbar) {}
		SLATE_ARGUMENT(TArray<FEditorTool*>, ToolsToRegister)
	SLATE_END_ARGS()

	SMapEditorToolbar();
	~SMapEditorToolbar();

	void Construct(const FArguments& InArgs);

	bool IsToolEnable() const;
	EVisibility GetToolVisibility() const;

	void CleanAll();

	class UWorld* GetWorld();
private:

	// Begin SWidget
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	// End SWidget

	void InitToolState();

	// ~Begin：Create Content Widget
	TSharedRef<SBorder> CreatePlaceStageObjectPanel();
	TSharedRef<SBorder> CreateEditorToolsPanel();
	TSharedRef<SWrapBox> CreateToolbarPanel();
	// ~End: Create Content Widget

	// ~Begin Placement Group
	TSharedRef<SWidget> CreatePlacementGroupTab( PlacementTabInfo Info );
	void OnPlacementTabChanged( ECheckBoxState NewState, FName CategoryName );
	const FSlateBrush* PlacementGroupBorderImage( FName CategoryName ) const;
	ECheckBoxState GetPlacementTabCheckedState( FName CategoryName ) const;
	void RefreshPlacementContent();
	void RegisterPlaceableStageObjec();
	TSharedRef<ITableRow> OnGenerateWidgetForItem(TSharedPtr<FPlacementStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	// ~End Placement Group

	// ~Begin: Editor Setting Tools
	TSharedRef<SWidget> CreateBaseMapSettings();
	TSharedRef<SWidget> CreateWeatherSettings();
	// ~End: Editor Setting Tools

	FReply OnOpenMapEditorSceneClicked();
	FReply OnCleanMapClicked();
	FReply OnCleanMapConfigObjectsClicked();
	FReply OnExportMapDataClicked();
	void OnMapChanged( UWorld* World, EMapChangeType MapChangeType );

	void InternalCleanMap(bool CleanLoadedMap, bool CleanConfigObjs);
	void InternalExportMapData();

	/** Select Map */
	FReply OnSelectMapClicked();
	void OnSelectMap(const TArray<FAssetData>& SelectedAssets);
	void OnSelectMapCancel();
	TSharedRef<SWidget> CreateMapPicker() = delete;

	/** Select Map Data File */
	FReply OnSelectMapDataClicked();
	void LoadMapDataForPreview(const FString& MapFile);

	/** Called when the search text changes */
	void OnSearchChanged(const FText& InFilterText);
	void OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);

private:
	/** Active tab of placement category */
	FName ActivePlacementCategory;
	bool bNeedRefreshPlacementContent;

	/** The Button show when current level is not "MapEditor" */
	TSharedPtr<SButton> OpenEditorSceneButton;

	/** Handler to observe map change event */
	FDelegateHandle OnMapChangedHandler;

	/** Main Panel which hold editor tools */
	TSharedPtr<SPanel> MainToolPanel;

	/** The search box used to update the filter text */
	TSharedPtr<SSearchBox> SearchBoxPtr;

	/** If pick map dialog is showing */
	bool bPickMapDialogOpend;

	TSharedPtr<SButton> PickMapButton;

	FString MapPackagedName;
	TSharedPtr<STextBlock> MapName;

	/** The Level we actually */
	class ULevel* DataLevel;

	FString MapDataFileName;
	TSharedPtr<STextBlock> TB_MapDataFileName;

	/** The current class to set new or added levels streaming method to. */
	UClass*				AddedLevelStreamingClass;

	typedef TArray<TSharedPtr<FPlacementStageObject>> PlaceableSet;
	TMap<FName, PlaceableSet> PlaceableObjects;
	PlaceableSet FilteredItems;
	TSharedPtr<SListView<TSharedPtr<FPlacementStageObject>>> PlacementListView;
	
	bool HasRegisterTools;

	void initEditorTool();

	UPROPERTY()
	class AMapEditorTool* EditorTool;
};