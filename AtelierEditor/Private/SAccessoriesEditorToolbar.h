// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: I-RUNG YU
// Date: 2018-11-29

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Styling/SlateColor.h"
#include "Input/Reply.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EditorTool.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "EditorStyleSet.h"
#include "AssetData.h"
#include "UnrealWidget.h"
#include "PreviewScene.h"
#include "EditorViewportClient.h"
#include "HitProxies.h"
#include "SEditorViewport.h"
#include "EditorAnimUtils.h"
#include "Framework/Text/SlateHyperlinkRun.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"
#include "System/Project.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Engine/DataTable.h"
#include "AccessoriesEidtorCharacter.h"

class SPanel;
class SButton;
class SWrapBox;
class SSplitter;
class STextBlock;
struct FAssetData;
class UActorFactory;
class USkeletalMesh;
class FAssetRegistryModule;
class IAssetRegistry;
enum class EMapChangeType : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogAtelierAccessoriesEditor, Log, All);

#define REGISTER_PLACEABLE_CATEGORY(CAT)\
	static FName Get##CAT##Name() { const FName Name = #CAT; return Name; }


//AccessoriesSelectPanel
class FItemCategory
{
public:
	REGISTER_PLACEABLE_CATEGORY(Accessories)
};

class ItemTabInfo 
{
public:
	ItemTabInfo(FName Name)
		: Name(Name) {}
	FName Name;
};

struct FItemStageObject
{
public:
	FItemStageObject(UActorFactory* InFactory, const FAssetData& InAssetData)
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


DECLARE_DELEGATE_TwoParams(FSelectItemEvent, bool, UStaticMesh*);
DECLARE_DELEGATE(FRemoveEvent);
class SItemStageObjectAssetEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SItemStageObjectAssetEntry) {}

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

		SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<const FItemStageObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;

	TSharedPtr<const FItemStageObject> Item;
	UStaticMesh *NewStatic;
	void SetSelectItemEventDelegate(FSelectItemEvent SelectDelegate);
	void SetRemoveDataDelegate(FRemoveEvent RemoveDelegate);
private:
	const FSlateBrush* GetBorder() const;

	bool bIsPressed;

	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;

	FSelectItemEvent SelectItemEventDelegate;
	FRemoveEvent RemoveItemEventDelegate;

	TSharedRef<SWrapBox> CreateRemoveDataPanel();

	FReply OnRemoveDataClicked();
};


//BodySelectPanel
class FBodyCategory
{
public:
	REGISTER_PLACEABLE_CATEGORY(Body)
};

class BodyTabInfo
{
public:
	BodyTabInfo(FName Name)
		: Name(Name) {}
	FName Name;
};

struct FBodyStageObject
{
public:
	FBodyStageObject(UActorFactory* InFactory, const FAssetData& InAssetData)
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

	/** The factory used to create an instance of this placeable Body */
	UActorFactory* Factory;

	/** Asset data pertaining to the class */
	FAssetData AssetData;

	/** This Body's display name */
	FText DisplayName;

	UStaticMesh *CurrentItemMesh = nullptr;
};


DECLARE_DELEGATE_TwoParams(FSelectBodyEvent, bool, USkeletalMesh*);
DECLARE_DELEGATE(FRemoveEvent);
class SBodyStageObjectAssetEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBodyStageObjectAssetEntry) {}

	/** Highlight this text in the text block */
	SLATE_ATTRIBUTE(FText, HighlightText)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<const FBodyStageObject>& InItem);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool IsPressed() const;
	
	
	TSharedPtr<const FBodyStageObject> Body;
	USkeletalMesh *NewSkeleton;
	UStaticMesh *NewStatic;
	void SetSelectBodyEventDelegate(FSelectBodyEvent SelectDelegate);
	void SetRemoveSingleDataDelegate(FRemoveEvent RemoveDelegate);
private:
	const FSlateBrush* GetBorder() const;

	bool bIsPressed;

	/** Brush resource that represents a button */
	const FSlateBrush* NormalImage;
	/** Brush resource that represents a button when it is hovered */
	const FSlateBrush* HoverImage;
	/** Brush resource that represents a button when it is pressed */
	const FSlateBrush* PressedImage;

	FSelectBodyEvent SelectBodyEventDelegate;

	FRemoveEvent RemoveBodyEventDelegate;

	EVisibility IsSetCurrentAccessoriesData(FText BodyName, UStaticMesh *CurrentItemMesh);

	TSharedRef<SWrapBox> CreateRemoveSingleDataPanel();
	FReply OnRemoveSingleDataClicked();

	FText CheckStyle(FText BodyName, UStaticMesh *CurrentItemMesh);
};

//EditorViewport
class SBasePoseViewport : public /*SCompoundWidget*/SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SBasePoseViewport)
	{}

	SLATE_ARGUMENT(USkeletalMesh*, Skeleton)
		SLATE_END_ARGS()

public:
	SBasePoseViewport();

	void Construct(const FArguments& InArgs);
	void SetSkeleton(USkeletalMesh* Skeleton);
	void SetStatic(UStaticMesh* Static);
	FTransform StaticMeshComponentTrans;
	class ASkeletalMeshActor* Actor;
	class USkeletalMeshComponent* HeadComponent;
	class USkeletalMeshComponent* BodyComponent;
	class USkeletalMeshComponent* PreviewComponent;
	class UStaticMeshComponent* MeshComponent;
	FPreviewScene PreviewScene;
	
protected:
	/** SEditorViewport interface */
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
private:
	/** Skeleton */
	USkeletalMesh * TargetSkeleton;
	UStaticMesh * TargetStatic;
	/** Viewport client */
	TSharedPtr<class FBasePoseViewportClient> ViewportClient;
	
	


	bool IsVisible() const override;
};




//MainPanel
class SAccessoriesEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAccessoriesEditorToolbar) 
		: _ShowPublicViewControl(false)
		, _HideNameArea(false)
		, _SetNotifyHook(true)
		, _ShowTitleArea(false)
		{}
		SLATE_ARGUMENT(TArray<FEditorTool*>,ToolsToRegister)
		SLATE_ARGUMENT(bool, ShowPublicViewControl)
		SLATE_ARGUMENT(bool, HideNameArea)
		SLATE_ARGUMENT(FIsPropertyEditingEnabled, IsPropertyEditingEnabledDelegate)
		SLATE_ARGUMENT(FOnFinishedChangingProperties::FDelegate, OnFinishedChangingProperties)
		SLATE_ARGUMENT(FName, ViewIdentifier)
		SLATE_ARGUMENT(bool, SetNotifyHook)
		SLATE_ARGUMENT(bool, ShowTitleArea)
	SLATE_END_ARGS()

	SAccessoriesEditorToolbar();
	~SAccessoriesEditorToolbar();

	void Construct(const FArguments& InArgs);


	AActor * newActorCreated = nullptr;
	
	/**
	* Old Skeleton that was mapped
	* This data is needed to prevent users from selecting same skeleton
	*/
	USkeletalMesh* OldSkeleton;
	/**
	* New Skeleton that they would like to map to
	*/
	USkeletalMesh* NewSkeletalMesh = nullptr;
	UStaticMesh* NewStaticMesh = nullptr;

	bool IsToolEnable() const;
	EVisibility GetToolVisibility() const;
	void SetSelectBodyEvent(bool bNeedRefresh, USkeletalMesh* newSkeleton);
	void SetSelectItemEvent(bool bNeedRefresh, UStaticMesh* newStatic);

	/** Options for ShowDetails */
	struct FShowDetailsOptions
	{
		FText ForcedTitle;
		bool bForceRefresh;
		bool bShowComponents;
		bool bHideFilterArea;

		FShowDetailsOptions()
			:ForcedTitle()
			, bForceRefresh(false)
			, bShowComponents(true)
			, bHideFilterArea(false)
		{}

		FShowDetailsOptions(const FText& InForcedTitle, bool bInForceRefresh = false)
			:ForcedTitle(InForcedTitle)
			, bForceRefresh(bInForceRefresh)
			, bShowComponents(true)
			, bHideFilterArea(false)
		{}
	};

	/** Used to control visibility of a property in the property window */
	bool IsPropertyVisible(const struct FPropertyAndParent& PropertyAndParent) const;

	

	/** Update the inspector window to show information on the supplied objects */
	void ShowDetailsForSingleObject(UStaticMesh* Object, const FShowDetailsOptions& Options);

private:

	// Begin SWidget
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SWidget

	void InitToolState();


	

	// ~Begin：Create Content Widget
	TSharedRef<SBorder> CreatePlaceStageObjectPanel();
	TSharedRef<SBorder> CreateBodyObjectPanel();
	TSharedRef<SWrapBox> CreateEditorToolsPanel();
	TSharedRef<SWrapBox> CreateToolbarPanel();
	/*TSharedRef<SBorder> CreateViewportPanel();*/
	TSharedPtr<SBasePoseViewport> SourceViewport;
	TSharedPtr<SBasePoseViewport> TargetViewport;
	TSharedPtr<SBodyStageObjectAssetEntry> BodyStageObjectAssetEntry;
	TSharedPtr<SItemStageObjectAssetEntry> ItemStageObjectAssetEntry;
	// ~End: Create Content Widget


	// ~Begin Placement Group
	TSharedRef<SWidget> CreatePlacementGroupTab(ItemTabInfo Info);
	void OnPlacementTabChanged(ECheckBoxState NewState, FName CategoryName);
	TSharedRef<SWidget> CreateBodyGroupTab(BodyTabInfo Info);
	void OnBodyTabChanged(ECheckBoxState NewState, FName CategoryName);
	const FSlateBrush* PlacementGroupBorderImage(FName CategoryName) const;
	const FSlateBrush* BodyGroupBorderImage(FName CategoryName) const;
	ECheckBoxState GetPlacementTabCheckedState(FName CategoryName) const;
	ECheckBoxState GetBodyTabCheckedState(FName CategoryName) const;
	void RefreshPlacementContent();
	void RegisterPlaceableStageObject();
	void RefreshBodyContentEvent();
	void RefreshBodyContent();
	void RefreshViewport();
	void RegisterBodyStageObject();
	TSharedRef<ITableRow> OnGenerateWidgetForItem(TSharedPtr<FItemStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateWidgetForBody(TSharedPtr<FBodyStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	// ~End Placement Group

	FReply OnCleanClicked();
	FReply OnSpawnActorClicked();
	FReply OnSelectAccessoriesClicked();
	FReply OnExportDataClicked();
	FReply OnExportCommonClicked();
	FReply OnResetClicked();
	void OnMapChanged(UWorld* World, EMapChangeType MapChangeType);

	void InternalExportData();
	void InternalExportCommonData();

	/** Select Map */
	FReply OnSelectMapClicked();
	void OnSelectMap(const TArray<FAssetData>& SelectedAssets);
	void OnSelectMapCancel();
	TSharedRef<SWidget> CreateMapPicker() = delete;


	/** Called when the search text changes */
	void OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);
	
private:
	/** Active tab of placement category */
	FName ActivePlacementCategory;
	bool bNeedRefreshPlacementContent;

	/** Active tab of Body category */
	FName ActiveBodyCategory;
	bool bNeedRefreshBodyContent;

	bool bNeedRefreshViewport;

	bool bNeedRefreshTransform;

	bool bStaticMeshSelected;

	bool bSkeletalMeshSelected;

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

	FString MapPackagedName;

	FString MapDataFileName;

	/** The current class to set new or added levels streaming method to. */
	UClass*	AddedLevelStreamingClass;

	typedef TArray<TSharedPtr<FItemStageObject>> PlaceableSet;
	TMap<FName, PlaceableSet> PlaceableObjects;
	PlaceableSet FilteredItems;
	TSharedPtr<SListView<TSharedPtr<FItemStageObject>>> PlacementListView;

	typedef TArray<TSharedPtr<FBodyStageObject>> BodySet;
	TMap<FName, BodySet> BodyObjects;
	BodySet FilteredBodys;
	TSharedPtr<SListView<TSharedPtr<FBodyStageObject>>> BodyListView;

	bool HasRegisterTools;

	FSelectBodyEvent TouchDelegate;
	FSelectItemEvent TouchItemDelegate;
	FRemoveEvent OnDataRemove;
	

protected:
	// Flags whether the slider is being moved at the moment on any of our widgets
	bool bIsUsingSlider;

	/** Add this property and all its child properties to SelectedObjectProperties */
	void AddPropertiesRecursive(UProperty* Property);

	/** User defined delegate for OnFinishedChangingProperties */
	FOnFinishedChangingProperties::FDelegate UserOnFinishedChangingProperties;

	/** String used as the title above the property window */
	FText PropertyViewTitle;

	/** Should we currently show the property view */
	bool bShowInspectorPropertyView;

	/** Should we currently show components */
	bool bShowComponents;

	/** Selected objects for this detail view */
	TArray< TWeakObjectPtr<UObject> > SelectedObjects;

	/** Component details customization enabled. */
	bool bComponenetDetailsCustomizationEnabled;

	/** Set of object properties that should be visible */
	TSet<TWeakObjectPtr<UProperty> > SelectedObjectProperties;

	/** User defined delegate for IsPropertyEditingEnabled: */
	FIsPropertyEditingEnabled IsPropertyEditingEnabledDelegate;

	/** Returns whether the properties in the view should be editable */
	bool IsPropertyEditingEnabled() const;

	/** Update the inspector window to show information on the supplied objects */
	void UpdateFromObjects(const TArray<UStaticMesh*>& PropertyObjects, struct FSelectionInfo& SelectionInfo, const FShowDetailsOptions& Options);

	/** When TRUE, the Kismet inspector needs to refresh the details view on Tick */
	bool bRefreshOnTick;

	/** Holds the property objects that need to be displayed by the inspector starting on the next tick */
	TArray<UStaticMesh*> RefreshPropertyObjects;

	/** Details options that are used by the inspector on the next refresh. */
	FShowDetailsOptions RefreshOptions;

	/** Border widget that wraps a dynamic context-sensitive widget for editing objects that the property window is displaying */
	TSharedPtr<SBorder> ContextualEditingBorderWidget;

	

	/** Property viewing widget */
	TSharedPtr<class IDetailsView> PropertyView;


};

