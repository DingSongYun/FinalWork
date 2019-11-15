// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: I-RUNG YU
// Date: 2018-11-29

#include "SAccessoriesEditorToolbar.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"
#include "Editor.h"
#include "SImage.h"
#include "SButton.h"
#include "SBoxPanel.h"
#include "SSplitter.h"
#include "SListView.h"
#include "STextBlock.h"
#include "Materials/Material.h"
#include "UnrealString.h"
#include "Framework/Application/SlateApplication.h"
#include "IAssetRegistry.h"
#include "LevelUtils.h"
#include "EditorStyle.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/EditorEngine.h"
#include "UnrealEdGlobals.h"
#include "EngineGlobals.h"
#include "ConstructorHelpers.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Settings/LevelEditorMiscSettings.h"
#include "ActorFactories/ActorFactory.h"
#include "GameModeInfoCustomizer.h"
#include "DeclarativeSyntaxSupport.h"
#include "Engine/LevelStreaming.h"
#include "EditorUtilities.h"
#include "EngineUtils.h"
#include "AssetDragDropOp.h"
#include "EditorLevelUtils.h"
#include "Model/MapDataModel.h"
#include "JsonObjectConverter.h"
#include "Stage/AreaBox.h"
#include "Stage/StageObject.h"
#include "LevelEditor.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "System/GAGameInstance.h"
#include "StageObjectSaveHandler.h"
#include "AssetRegistryModule.h"
#include "AssetSelection.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Utilities/Utilities.h"
#include "System/CharacterManager.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Settings/SkeletalMeshEditorSettings.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "SEditorViewportToolBarMenu.h"
#include "STransformViewportToolbar.h"
#include "EditorViewportCommands.h"
#include "SlateTypes.h"
#include "IDetailCustomization.h"
#include "Settings/EditorExperimentalSettings.h"
#include "Components/ActorComponent.h"
#include "Components/ChildActorComponent.h"
#include "DataTables/Accessories.h"
#include "EditorModeManager.h"
#include "Animation/SkeletalMeshActor.h"
#include "ScopedTransaction.h"
#include "Dialogs/Dialogs.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "System/Project.h"



#define LOCTEXT_NAMESPACE "IntervalStructCustomization"
DEFINE_LOG_CATEGORY(LogAtelierAccessoriesEditor)

/*****************************************************************/
/******************SItemStageObjectAssetEntry*********************/
/*****************************************************************/
void SItemStageObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FItemStageObject>& InItem)
{
	
	bIsPressed = false;

	Item = InItem;

	TSharedPtr< SHorizontalBox > ActorType = SNew(SHorizontalBox);

	const bool bIsClass = Item->AssetData.GetClass() == UClass::StaticClass();
	const bool bIsActor = bIsClass ? CastChecked<UClass>(Item->AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

	AActor* DefaultActor = nullptr;
	if (Item->Factory != nullptr)
	{
		DefaultActor = Item->Factory->GetDefaultActor(Item->AssetData);
	}
	else if (bIsActor)
	{
		DefaultActor = CastChecked<AActor>(CastChecked<UClass>(Item->AssetData.GetAsset())->ClassDefaultObject);
	}

	UClass* DocClass = nullptr;
	TSharedPtr<IToolTip> AssetEntryToolTip;
	if (DefaultActor != nullptr)
	{
		DocClass = DefaultActor->GetClass();
		AssetEntryToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
	}

	if (!AssetEntryToolTip.IsValid())
	{
		AssetEntryToolTip = FSlateApplicationBase::Get().MakeToolTip(Item->DisplayName);
	}

	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");

	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;

	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(this, &SItemStageObjectAssetEntry::GetBorder)
			.Cursor(EMouseCursor::GrabHand)
			.ToolTip(AssetEntryToolTip)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.Padding(0)
					.AutoWidth()
					[
						// Drop shadow border
						SNew(SBorder)
						.Padding(4)
						.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
						[
							SNew(SBox)
							.WidthOverride(35)
							.HeightOverride(35)
						]
					]

				+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(2, 0, 4, 0)
					[
						SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(0, 0, 0, 1)
							.AutoHeight()
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
								.Text(Item->DisplayName)
								.HighlightText(InArgs._HighlightText)
							]
							
					]
				+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(2, 0, 4, 0)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(0, 0, 0, 1)
						.AutoHeight()
						[
							CreateRemoveDataPanel()
						]
					]
			]
		];
}

const FSlateBrush* SItemStageObjectAssetEntry::GetBorder() const
{
	if (IsPressed())
	{
		return PressedImage;
	}
	else if (IsHovered())
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

FReply SItemStageObjectAssetEntry::OnRemoveDataClicked()
{
	FString PackagePath = FString("/Game/Data/DataTables/Accessories");
	UPackage *Package = CreatePackage(nullptr, *PackagePath);
	UDataTable* DataTable;
	FName ItemName = FName(*Item->DisplayName.ToString());
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
	if (DataTable != nullptr)
	{
		TArray<FAccessories*> OutAllRows;
		FString Context; // Used in error reporting.
		DataTable->GetAllRows<FAccessories>(Context, OutAllRows); // Populates the array with all the data from the CSV.
		TArray<FName> key = DataTable->GetRowNames();
		if (key.Contains(ItemName)) {
			DataTable->RemoveRow(ItemName);
		}
		else
		{
			auto Message = NSLOCTEXT("UnrealClient", "Error", "表中不存在此字段");
			FNotificationInfo Info(Message);
			Info.bFireAndForget = true;
			Info.ExpireDuration = 5.0f;
			Info.bUseSuccessFailIcons = false;
			Info.bUseLargeFont = false;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		FString AssetPath = FString("D:/Game/atelier/client-ue4/Content/Data/DataTables/");
		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("Accessories"), *FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
	}
	RemoveItemEventDelegate.ExecuteIfBound();
	return FReply::Handled();
}

TSharedRef<SWrapBox> SItemStageObjectAssetEntry::CreateRemoveDataPanel()
{
	TSharedRef<SWrapBox> RemovePanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	RemovePanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				//////////////////////////
				//   RemoveData Button  //
				//////////////////////////
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SItemStageObjectAssetEntry::OnRemoveDataClicked)
					.ContentPadding(FMargin(6, 2))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
							SNew(SHorizontalBox)
							// Icon
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
								[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
								.Text(FEditorFontGlyphs::Trash)
								]
							// Import Text
							+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(4, 0, 0, 0)
								[
									SNew(STextBlock)
									.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
									.Text(FText::FromString("RemoveData"))
								]
					]
				]
			]
		];

	return RemovePanel;
}

void SItemStageObjectAssetEntry::SetSelectItemEventDelegate(FSelectItemEvent SelectDelegate)
{
	SelectItemEventDelegate = SelectDelegate;
}

void SItemStageObjectAssetEntry::SetRemoveDataDelegate(FRemoveEvent RemoveDelegate)
{
	RemoveItemEventDelegate = RemoveDelegate;
}

bool SItemStageObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

FReply SItemStageObjectAssetEntry::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	if (FEditorDelegates::OnAssetDragStarted.IsBound())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add(Item->AssetData);
		FEditorDelegates::OnAssetDragStarted.Broadcast(DraggedAssetDatas, Item->Factory);
		return FReply::Handled();
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(Item->AssetData, Item->Factory));

	}
	else
	{
		return FReply::Handled();
	}
}

FReply SItemStageObjectAssetEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;

		return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
	}

	return FReply::Unhandled();
}

FReply SItemStageObjectAssetEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UE_LOG(LogAtelierAccessoriesEditor, Log, TEXT("Item be Selected: %s"), *Item->AssetData.PackageName.ToString());
		FARFilter MFilter;
		MFilter.ClassNames.Add(UStaticMesh::StaticClass()->GetFName());
		MFilter.bRecursivePaths = true;
		TArray< FAssetData > MeshAssetList;
		AssetRegistry.GetAssets(MFilter, MeshAssetList);
		FString AssetPathContain = Item->DisplayName.ToString();

		for (FAssetData Asset : MeshAssetList)
		{
			if (Asset.PackageName.ToString().Contains(*AssetPathContain))
			{
				NewStatic = const_cast<UStaticMesh*>(Cast<UStaticMesh>(Asset.GetAsset()));
			}
		}
		SelectItemEventDelegate.ExecuteIfBound(true, NewStatic);
		bIsPressed = false;
	}
	return FReply::Unhandled();
}


/*****************************************************************/
/******************SBodyStageObjectAssetEntry*********************/
/*****************************************************************/
void SBodyStageObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FBodyStageObject>& InItem)
{
	bIsPressed = false;

	Body = InItem;
	NewStatic = Body->CurrentItemMesh;

	TSharedPtr< SHorizontalBox > ActorType = SNew(SHorizontalBox);

	const bool bIsClass = Body->AssetData.GetClass() == UClass::StaticClass();
	const bool bIsActor = bIsClass ? CastChecked<UClass>(Body->AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

	AActor* DefaultActor = nullptr;
	if (Body->Factory != nullptr)
	{
		DefaultActor = Body->Factory->GetDefaultActor(Body->AssetData);
	}
	else if (bIsActor)
	{
		DefaultActor = CastChecked<AActor>(CastChecked<UClass>(Body->AssetData.GetAsset())->ClassDefaultObject);
	}

	UClass* DocClass = nullptr;
	TSharedPtr<IToolTip> AssetEntryToolTip;
	if (DefaultActor != nullptr)
	{
		DocClass = DefaultActor->GetClass();
		AssetEntryToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
	}

	if (!AssetEntryToolTip.IsValid())
	{
		AssetEntryToolTip = FSlateApplicationBase::Get().MakeToolTip(Body->DisplayName);
	}

	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");

	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;
	

	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(this, &SBodyStageObjectAssetEntry::GetBorder)
				.Cursor(EMouseCursor::GrabHand)
				.ToolTip(AssetEntryToolTip)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
						.Padding(0)
						.AutoWidth()
						[
							// Drop shadow border
							SNew(SBorder)
							.Padding(4)
							.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
							[
								SNew(SBox)
								.WidthOverride(35)
								.HeightOverride(35)
							]
						]

					+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(2, 0, 4, 0)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
								.Padding(0, 0, 0, 1)
								.AutoHeight()
								[
									SNew(STextBlock)
										.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
										.Text(Body->DisplayName)
										.HighlightText(InArgs._HighlightText)
								]
						]
					+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(this->CheckStyle(Body->DisplayName, Body->CurrentItemMesh))
							.Visibility(this->IsSetCurrentAccessoriesData(Body->DisplayName,Body->CurrentItemMesh))
						]
					+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(2, 0, 4, 0)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(0, 0, 0, 1)
							.AutoHeight()
							[
								CreateRemoveSingleDataPanel()
							]
						]
				]
		];
}

const FSlateBrush* SBodyStageObjectAssetEntry::GetBorder() const
{
	if (IsPressed())
	{
		return PressedImage;
	}
	else if (IsHovered())
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

EVisibility SBodyStageObjectAssetEntry::IsSetCurrentAccessoriesData(FText BodyName, UStaticMesh *CurrentItemMesh)
{
	if (!CurrentItemMesh) {
		return EVisibility::Hidden;
	}

	UDataTable* DataTable;
	FString Context;
	FAccessories row;
	FTransform LoadTransform;
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
	if (DataTable->GetRowNames().Num())
	{
		TArray<FName> RowNames = DataTable->GetRowNames();
		if (RowNames.Contains(*CurrentItemMesh->GetName())) {
			int32 TransMapLength = DataTable->FindRow<FAccessories>(*CurrentItemMesh->GetName(), Context)->TransMap.Num();
			if (TransMapLength)
			{
					return EVisibility::Visible;
			}
		}
	}
	return EVisibility::Hidden;
}

TSharedRef<SWrapBox> SBodyStageObjectAssetEntry::CreateRemoveSingleDataPanel()
{
	TSharedRef<SWrapBox> RemovePanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	RemovePanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)
			//////////////////////////
			//   RemoveData Button  //
			//////////////////////////
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.OnClicked(this, &SBodyStageObjectAssetEntry::OnRemoveSingleDataClicked)
		.ContentPadding(FMargin(6, 2))
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
		[
			SNew(SHorizontalBox)
			// Icon
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
		.Text(FEditorFontGlyphs::Trash)
		]
	// Import Text
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 0, 0)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Text(FText::FromString("RemoveData"))
		]
		]
		]
		]
		];

	return RemovePanel;
}

FReply SBodyStageObjectAssetEntry::OnRemoveSingleDataClicked()
{
	FString PackagePath = FString("/Game/Data/DataTables/Accessories");
	UPackage *Package = CreatePackage(nullptr, *PackagePath);
	UDataTable* DataTable;
	FName BodyName = FName(*Body->DisplayName.ToString());
	FName ItemName = *NewStatic->GetName();
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
	if (DataTable != nullptr)
	{
		TArray<FAccessories*> OutAllRows;
		FString Context; // Used in error reporting.
		DataTable->GetAllRows<FAccessories>(Context, OutAllRows); // Populates the array with all the data from the CSV.
		TArray<FName> key = DataTable->GetRowNames();
		if (key.Contains(ItemName)) {
			TMap<FName,FTransform>ItemMap = DataTable->FindRow<FAccessories>(ItemName, Context)->TransMap;
			if (ItemMap.Contains(BodyName)) {
				DataTable->FindRow<FAccessories>(ItemName, Context)->TransMap.FindAndRemoveChecked(BodyName);
			}
			else{
				auto Message = NSLOCTEXT("UnrealClient", "Error", "此Item不存在对应的Body");
				FNotificationInfo Info(Message);
				Info.bFireAndForget = true;
				Info.ExpireDuration = 5.0f;
				Info.bUseSuccessFailIcons = false;
				Info.bUseLargeFont = false;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
		else
		{
			auto Message = NSLOCTEXT("UnrealClient", "Error", "表中不存在此Item");
			FNotificationInfo Info(Message);
			Info.bFireAndForget = true;
			Info.ExpireDuration = 5.0f;
			Info.bUseSuccessFailIcons = false;
			Info.bUseLargeFont = false;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		FString AssetPath = FString("D:/Game/atelier/client-ue4/Content/Data/DataTables/");
		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("Accessories"), *FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
	}
	RemoveBodyEventDelegate.ExecuteIfBound();
	return FReply::Handled();
}

FText SBodyStageObjectAssetEntry::CheckStyle(FText BodyName, UStaticMesh *CurrentItemMesh)
{
	if (!CurrentItemMesh) {
		return FEditorFontGlyphs::Check_Square_O;
	}

	UDataTable* DataTable;
	FString Context;
	FAccessories row;
	FTransform LoadTransform;
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/Accessories.Accessories'"));
	if (DataTable->GetRowNames().Num())
	{
		TArray<FName> RowNames = DataTable->GetRowNames();
		if (RowNames.Contains(*CurrentItemMesh->GetName())) {
			int32 TransMapLength = DataTable->FindRow<FAccessories>(*CurrentItemMesh->GetName(), Context)->TransMap.Num();
			if (TransMapLength)
			{
				TMap<FName, FTransform> map = DataTable->FindRow<FAccessories>(*CurrentItemMesh->GetName(), Context)->TransMap;
				if (map.Contains(FName(*BodyName.ToString()))) {
					return FEditorFontGlyphs::Check_Square;
				}
				else
				{
					return FEditorFontGlyphs::Check_Square_O;
				}
			}
		}
	}

	return FEditorFontGlyphs::Check_Square_O;
}

void SBodyStageObjectAssetEntry::SetSelectBodyEventDelegate(FSelectBodyEvent SelectDelegate)
{
	SelectBodyEventDelegate = SelectDelegate;
}

void SBodyStageObjectAssetEntry::SetRemoveSingleDataDelegate(FRemoveEvent RemoveDelegate)
{
	RemoveBodyEventDelegate = RemoveDelegate;
}

bool SBodyStageObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

FReply SBodyStageObjectAssetEntry::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	if (FEditorDelegates::OnAssetDragStarted.IsBound())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add(Body->AssetData);
		FEditorDelegates::OnAssetDragStarted.Broadcast(DraggedAssetDatas, Body->Factory);
		return FReply::Handled();
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(Body->AssetData, Body->Factory));
	}
	else
	{
		return FReply::Handled();
	}
}

FReply SBodyStageObjectAssetEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;

		return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
	}

	return FReply::Unhandled();
}

FReply SBodyStageObjectAssetEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UE_LOG(LogAtelierAccessoriesEditor, Log, TEXT("Body be Selected: %s"), *Body->AssetData.PackageName.ToString());
		FARFilter MFilter;
		MFilter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
		MFilter.bRecursivePaths = true;
		TArray< FAssetData > MeshAssetList;
		AssetRegistry.GetAssets(MFilter, MeshAssetList);
		FString AssetPathContain = Body->DisplayName.ToString();

		for (FAssetData Asset : MeshAssetList)
		{
			if (Asset.PackageName.ToString().Contains(*AssetPathContain))
			{
				NewSkeleton = const_cast<USkeletalMesh*>(Cast<USkeletalMesh>(Asset.GetAsset()));
			}
		}
		SelectBodyEventDelegate.ExecuteIfBound(true, NewSkeleton);
		bIsPressed = false;
	}
	return FReply::Unhandled();
}



//////////////////////////////////////////////////////////////////////////
// FKismetSelectionInfo

struct FSelectionInfo
{
public:
	TArray<UActorComponent*> EditableComponentTemplates;
	TArray<UObject*> ObjectsForPropertyEditing;
};


/*****************************************************************/
/********************SAccessoriesEditorToolbar********************/
/*****************************************************************/
SAccessoriesEditorToolbar::SAccessoriesEditorToolbar()
{
	const TSubclassOf<ULevelStreaming> DefaultLevelStreamingClass = GetDefault<ULevelEditorMiscSettings>()->DefaultLevelStreamingClass;
	if (DefaultLevelStreamingClass)
	{
		AddedLevelStreamingClass = DefaultLevelStreamingClass;
	}

	RegisterPlaceableStageObject();
	RegisterBodyStageObject();
	InitToolState();

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	OnMapChangedHandler = LevelEditorModule.OnMapChanged().AddRaw(this, &SAccessoriesEditorToolbar::OnMapChanged);
}

SAccessoriesEditorToolbar::~SAccessoriesEditorToolbar()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnMapChanged().Remove(OnMapChangedHandler);
}

void SAccessoriesEditorToolbar::InitToolState()
{
	bPickMapDialogOpend = false;
	MapPackagedName = "";
	ActivePlacementCategory = "";
	ActiveBodyCategory = "";
	HasRegisterTools = false;
}

FReply SAccessoriesEditorToolbar::OnSpawnActorClicked()
{
	for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
	{
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
		if (EditorChar) {
			auto Message = NSLOCTEXT("UnrealClient", "Error", "场景中已存在Actor");
			FNotificationInfo Info(Message);
			Info.bFireAndForget = true;
			Info.ExpireDuration = 5.0f;
			Info.bUseSuccessFailIcons = false;
			Info.bUseLargeFont = false;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else {
			FVector objectPosition(0.f, 0.f, 143.0f);
			FRotator objectRotation(0.f, 0.f, 0.f); //in degrees
			FVector objectScale(1, 1, 1);
			FTransform objectTrasform(objectRotation, objectPosition, objectScale);
			UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
			ULevel * currentLevel = currentWorld->GetLevel(0);
			UClass * ActorClass = AAccessoriesEidtorCharacter::StaticClass();
			newActorCreated = GEditor->AddActor(currentLevel, ActorClass, objectTrasform, true, RF_Public | RF_Standalone | RF_Transactional);

			GEditor->EditorUpdateComponents();
			currentWorld->UpdateWorldComponents(true, false);
			GLevelEditorModeTools().MapChangeNotify();
		}
	}
	/*if (newActorCreated) {
		
	}
	else {
		
	}*/
	return FReply::Handled();
}

FReply SAccessoriesEditorToolbar::OnSelectAccessoriesClicked()
{
	if (newActorCreated) {
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
		if (EditorChar) {
			EditorChar->SelectAccessoriesComponent();
		}
	}
	else {
		for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
		{
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
			if (EditorChar) {
				EditorChar->SelectAccessoriesComponent();
			}
		}
	}
	return FReply::Handled();
}


void SAccessoriesEditorToolbar::Construct(const FArguments& InArgs)
{

	FEditorUtilities::OpenLevelByName("AccessoriesEditorMap");
	
	//Delegate Bind 
	TouchDelegate.BindSP(this, &SAccessoriesEditorToolbar::SetSelectBodyEvent);
	TouchItemDelegate.BindSP(this, &SAccessoriesEditorToolbar::SetSelectItemEvent);
	OnDataRemove.BindSP(this, &SAccessoriesEditorToolbar::RefreshBodyContentEvent);
	
	bSkeletalMeshSelected = false;
	bStaticMeshSelected = false;
	bRefreshOnTick = false;

	//RegisterPlaceableStageObject();
	ChildSlot
	[  
	SNew(SVerticalBox)
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateEditorToolsPanel()
		]
	+ SVerticalBox::Slot()
		.FillHeight(1.0)
		[
		SAssignNew(MainToolPanel, SVerticalBox)
		+ SVerticalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0, 5, 0, 0)
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
					.Value(0.25f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0)
						[
							CreatePlaceStageObjectPanel()
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0)
						[
							CreateBodyObjectPanel()
						]
					]
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateToolbarPanel()
			]
		]
	];

	TArray<UStaticMesh*> InitialSelectedObjects;
	FSelectionInfo SelectionInfo;
	UpdateFromObjects(InitialSelectedObjects, SelectionInfo, SAccessoriesEditorToolbar::FShowDetailsOptions(FText::GetEmpty(), true));

}


void SAccessoriesEditorToolbar::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bNeedRefreshPlacementContent)
	{
		RefreshPlacementContent();
	}
	if (bNeedRefreshBodyContent)
	{
		RefreshBodyContent();
	}
	if (bNeedRefreshViewport)
	{
		RefreshViewport();
	}
	if (bRefreshOnTick)
	{
		FSelectionInfo SelectionInfo;
		UpdateFromObjects(RefreshPropertyObjects, SelectionInfo, RefreshOptions);
		RefreshPropertyObjects.Empty();
		bRefreshOnTick = false;
	}
}





/** Update the inspector window to show information on the supplied object */
void SAccessoriesEditorToolbar::ShowDetailsForSingleObject(UStaticMesh* Object, const FShowDetailsOptions& Options)
{
	static bool bIsReentrant = false;
	if (!bIsReentrant)
	{
		bIsReentrant = true;
		// When the selection is changed, we may be potentially actively editing a property,
		// if this occurs we need need to immediately clear keyboard focus
		if (FSlateApplication::Get().HasFocusedDescendants(AsShared()))
		{
			FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Mouse);
		}
		bIsReentrant = false;
	}
	TArray<UStaticMesh*> PropertyObjects;
	if (Object != NULL)
	{
		PropertyObjects.Add(Object);
	}
	// Refresh is being deferred until the next tick, this prevents batch operations from bombarding the details view with calls to refresh
	RefreshPropertyObjects = PropertyObjects;
	RefreshOptions = Options;
	bRefreshOnTick = true;
}


/*****************************************************/
/***************CreatePanelFunction*******************/
/*****************************************************/

TSharedRef<SWrapBox> SAccessoriesEditorToolbar::CreateToolbarPanel()//Button on the bottom (Clean)
{
	TSharedRef<SWrapBox> ToolBarPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	ToolBarPanel->AddSlot()
		.FillLineWhenWidthLessThan(600)
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
			SNew(SHorizontalBox)

					//////////////////////////
					//     Export Button     //
					//////////////////////////
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
								SNew(SButton)
								.ButtonStyle(FEditorStyle::Get(), "FlatButton")
								.OnClicked(this, &SAccessoriesEditorToolbar::OnExportDataClicked)
								.ContentPadding(FMargin(6, 2))
								.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
								[
									SNew(SHorizontalBox)
									// Icon
									+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.AutoWidth()
										[
											SNew(STextBlock)
											.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
											.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
											.Text(FEditorFontGlyphs::Check_Circle)
										]
									// Import Text
									+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(4, 0, 0, 0)
										[
											SNew(STextBlock)
											.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
											.Text(FText::FromString("SaveAccessoriesData"))
										]
								]
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
								SNew(SButton)
								.ButtonStyle(FEditorStyle::Get(), "FlatButton")
								.OnClicked(this, &SAccessoriesEditorToolbar::OnExportCommonClicked)
								.ContentPadding(FMargin(6, 2))
								.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
								[
									SNew(SHorizontalBox)
									// Icon
									+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.AutoWidth()
									[
										SNew(STextBlock)
										.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
										.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
										.Text(FEditorFontGlyphs::Check_Circle)
									]
								// Import Text
								+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(4, 0, 0, 0)
									[
										SNew(STextBlock)
										.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
										.Text(FText::FromString("SaveCommonData"))
									]
								]
						]
			]
		];
	return ToolBarPanel;
}

TSharedRef<SBorder> SAccessoriesEditorToolbar::CreatePlaceStageObjectPanel()
{
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	TSharedRef<SVerticalBox> Tabs = SNew(SVerticalBox);
	for (auto& Element : PlaceableObjects)
	{
		Tabs->AddSlot()
			.AutoHeight()
			[
				CreatePlacementGroupTab(ItemTabInfo(Element.Key))
			];
	}
	TSharedRef<SBorder> PlaceStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
				.Padding(0)
				[	
					//Tab Pannel
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							Tabs
						]
					//item
					+ SHorizontalBox::Slot()
						[
							//////
							SNew(SBorder)
							.Padding(FMargin(3))
							.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
							[
								//////
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.FillWidth(1.0f)
										[
											SAssignNew(PlacementListView, SListView<TSharedPtr<FItemStageObject>>)
											.ListItemsSource(&FilteredItems)
											.OnGenerateRow(this, &SAccessoriesEditorToolbar::OnGenerateWidgetForItem)
											.ExternalScrollbar(ScrollBar)
										]
									+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											ScrollBar
										]
								]
								//////
							]
							//////
						]
						//////
				]
		];

	return PlaceStageObjectPanel;
}

TSharedRef<SBorder> SAccessoriesEditorToolbar::CreateBodyObjectPanel()
{
	//ScrollBar
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	//Tabs
	TSharedRef<SVerticalBox> Tabs = SNew(SVerticalBox);
	for (auto& Element : BodyObjects)
	{
		Tabs->AddSlot()
			.AutoHeight()
			[
				CreateBodyGroupTab(BodyTabInfo(Element.Key))
			];
	}
	TSharedRef<SBorder> BodyStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

	+ SVerticalBox::Slot()
		.Padding(0)
		[
			//Tab Pannel
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
				[
					Tabs
				]
			//Body
			+ SHorizontalBox::Slot()
				[
					SNew(SBorder)
						.Padding(FMargin(3))
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	
						[
							//////
							SNew(SOverlay)
							+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.FillWidth(1.0f)
										[
											SAssignNew(BodyListView, SListView<TSharedPtr<FBodyStageObject>>)
											.ListItemsSource(&FilteredBodys)
											.OnGenerateRow(this, &SAccessoriesEditorToolbar::OnGenerateWidgetForBody)
											.ExternalScrollbar(ScrollBar)
										]
									+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											ScrollBar
										]
								]
			 	     	]
				]
				]
		];

	return BodyStageObjectPanel;
}

TSharedRef<SWrapBox> SAccessoriesEditorToolbar::CreateEditorToolsPanel()
{
	TSharedRef<SWrapBox> EditorToolPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	EditorToolPanel->AddSlot()
		.FillLineWhenWidthLessThan(600)
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)

			//////////////////////////
			//     SpawnActor Button     //
			//////////////////////////
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.OnClicked(this, &SAccessoriesEditorToolbar::OnSpawnActorClicked)
		.ContentPadding(FMargin(6, 2))
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
		[
			SNew(SHorizontalBox)
			// Icon
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
		.Text(FEditorFontGlyphs::Dropbox)
		]
	// Import Text
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 0, 0)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Text(FText::FromString("SpawnActor"))
		]
		]
		]

			//////////////////////////
			//     Select Button     //
			//////////////////////////
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.OnClicked(this, &SAccessoriesEditorToolbar::OnSelectAccessoriesClicked)
		.ContentPadding(FMargin(6, 2))
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
		[
			SNew(SHorizontalBox)
			// Icon
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
		.Text(FEditorFontGlyphs::Eye)
		]
	// Import Text
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 0, 0)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
		.Text(FText::FromString("SelectAccessories"))
		]
		]
		]
		]
		];

	return EditorToolPanel;
}

TSharedRef<SWidget> SAccessoriesEditorToolbar::CreatePlacementGroupTab(ItemTabInfo Info)
{
	return SNew(SCheckBox)
		.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
		.OnCheckStateChanged(this, &SAccessoriesEditorToolbar::OnPlacementTabChanged, Info.Name)
		.IsChecked(this, &SAccessoriesEditorToolbar::GetPlacementTabCheckedState, Info.Name)
		[
			SNew(SOverlay)
	+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(FVector2D(1, 30))
		]
	+ SOverlay::Slot()
		.Padding(FMargin(6, 0, 15, 0))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Tab.Text")
		.Text(FText::FromName(Info.Name))
		]
	+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(this, &SAccessoriesEditorToolbar::PlacementGroupBorderImage, Info.Name)
		]
		];
}

TSharedRef<SWidget> SAccessoriesEditorToolbar::CreateBodyGroupTab(BodyTabInfo Info)
{
	return SNew(SCheckBox)
		.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
		.OnCheckStateChanged(this, &SAccessoriesEditorToolbar::OnBodyTabChanged, Info.Name)
		.IsChecked(this, &SAccessoriesEditorToolbar::GetBodyTabCheckedState, Info.Name)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(FVector2D(1, 30))
		]
	+ SOverlay::Slot()
		.Padding(FMargin(6, 0, 15, 0))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Tab.Text")
		.Text(FText::FromName(Info.Name))
		]
	+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(this, &SAccessoriesEditorToolbar::BodyGroupBorderImage, Info.Name)
		]
		];
}

///////////

void SAccessoriesEditorToolbar::OnPlacementTabChanged(ECheckBoxState NewState, FName CategoryName)
{
	if (NewState == ECheckBoxState::Checked)
	{
		ActivePlacementCategory = CategoryName;
		bNeedRefreshPlacementContent = true;
	}
}

void SAccessoriesEditorToolbar::OnBodyTabChanged(ECheckBoxState NewState, FName CategoryName)
{
	if (NewState == ECheckBoxState::Checked)
	{
		ActiveBodyCategory = CategoryName;
		bNeedRefreshBodyContent = true;
	}
}

///////////

const FSlateBrush* SAccessoriesEditorToolbar::PlacementGroupBorderImage(FName CategoryName) const
{
	if (ActivePlacementCategory == CategoryName)
	{
		static FName PlacementBrowserActiveTabBarBrush("PlacementBrowser.ActiveTabBar");
		return FEditorStyle::GetBrush(PlacementBrowserActiveTabBarBrush);
	}
	else
	{
		return nullptr;
	}
}

const FSlateBrush* SAccessoriesEditorToolbar::BodyGroupBorderImage(FName CategoryName) const
{
	if (ActiveBodyCategory == CategoryName)
	{
		static FName BodyBrowserActiveTabBarBrush("PlacementBrowser.ActiveTabBar");
		return FEditorStyle::GetBrush(BodyBrowserActiveTabBarBrush);
	}
	else
	{
		return nullptr;
	}
}

///////////

ECheckBoxState SAccessoriesEditorToolbar::GetPlacementTabCheckedState(FName CategoryName) const
{
	return ActivePlacementCategory == CategoryName ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SAccessoriesEditorToolbar::GetBodyTabCheckedState(FName CategoryName) const
{
	return ActiveBodyCategory == CategoryName ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

///////////

void SAccessoriesEditorToolbar::RefreshPlacementContent()
{
	if (!bNeedRefreshPlacementContent)
		return;

	bNeedRefreshPlacementContent = false;

	if (PlaceableObjects.Contains(ActivePlacementCategory))
	{
		FilteredItems = PlaceableObjects[ActivePlacementCategory];
	}
	PlacementListView->RequestListRefresh();
}



void SAccessoriesEditorToolbar::RefreshBodyContent()
{
	if (!bNeedRefreshBodyContent)
		return;

	bNeedRefreshBodyContent = false;

	if (BodyObjects.Contains(ActiveBodyCategory))
	{
		FilteredBodys = BodyObjects[ActiveBodyCategory];
		for (const TSharedPtr<FBodyStageObject>& Body : FilteredBodys)
		{
			Body->CurrentItemMesh = NewStaticMesh;
		}
	}
	BodyListView->RebuildList();
}


void SAccessoriesEditorToolbar::RefreshViewport()
{
	if (!bNeedRefreshViewport)
		return;


	bNeedRefreshViewport = false;

	
	if (bSkeletalMeshSelected && bStaticMeshSelected)
	{
		
		if (newActorCreated) {
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
			EditorChar->LoadData();
		}
		else {
			for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
			{
				AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
				EditorChar->LoadData();
			}
		}
	}


	UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("Viewport Refresh"));
}


///////////

void SAccessoriesEditorToolbar::RegisterPlaceableStageObject()
{
	PlaceableObjects.Empty();
	PlaceableObjects.Add(FItemCategory::GetAccessoriesName(), PlaceableSet());

	auto CheckAssetDuplicate = [&](PlaceableSet& Set, TSharedPtr<FItemStageObject> Obj) -> bool
	{
		return Set.ContainsByPredicate([&](TSharedPtr<FItemStageObject>& pStageObject)
		{
			if (!Obj.IsValid() || !pStageObject.IsValid())
			{
				return false;
			}
			return Obj->AssetData == pStageObject->AssetData;
		});
	};

	auto CheckAssetClass = [&](UClass* Class, FAssetData AssetData, bool Force = false)
	{
		if (Class == nullptr)
		{
			return;
		}

		if ((!Class->HasAllClassFlags(CLASS_NotPlaceable) &&
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
			Class->ImplementsInterface(UStageObject::StaticClass())
			// && Class->ClassGeneratedBy == nullptr // for only c++ class
			)
			|| Force)
		{
			UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("RegisterStageObject:%s"), *Class->GetName());
			UActorFactory* Factory = GEditor->FindActorFactoryByClassForActorClass(UActorFactory::StaticClass(), Class);
			TSharedPtr<FItemStageObject> Obj = MakeShareable(new FItemStageObject(Factory, AssetData));
			PlaceableSet* TargetSet = nullptr;

			TargetSet = &PlaceableObjects[FItemCategory::GetAccessoriesName()];
			if (TargetSet != nullptr && !CheckAssetDuplicate(*TargetSet, Obj))
			{
				TargetSet->Add(Obj);
			}
		}
	};

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	//Find ALL Accessories(StaticMesh)
	FARFilter MFilter;
	MFilter.ClassNames.Add(UStaticMesh::StaticClass()->GetFName());
	MFilter.bRecursivePaths = true;
	TArray< FAssetData > MeshAssetList;
	AssetRegistry.GetAssets(MFilter, MeshAssetList);
	FString MeshAssetPath = "/Game/Assets/_Art/_Character/_Common/Accessory/";

	for (FAssetData Asset : MeshAssetList)
	{		
		if (Asset.PackageName.ToString().Contains(*MeshAssetPath))
		{
			UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("Detect Asset List:%s"), *Asset.PackageName.ToString());
			if (UObject* Obj = Asset.GetAsset()) {
				CheckAssetClass(Obj->StaticClass(), Asset, true);
			}
		}
	}
}

void SAccessoriesEditorToolbar::RefreshBodyContentEvent()
{
	bNeedRefreshBodyContent = true;
}

void SAccessoriesEditorToolbar::RegisterBodyStageObject()
{
	BodyObjects.Empty();
	BodyObjects.Add(FBodyCategory::GetBodyName(), BodySet());

	auto CheckAssetDuplicate = [&](BodySet& Set, TSharedPtr<FBodyStageObject> Obj) -> bool
	{
		return Set.ContainsByPredicate([&](TSharedPtr<FBodyStageObject>& pStageObject)
		{
			if (!Obj.IsValid() || !pStageObject.IsValid())
			{
				return false;
			}
			return Obj->AssetData == pStageObject->AssetData;
		});
	};

	auto CheckAssetClass = [&](UClass* Class, FAssetData AssetData, bool Force = false)
	{
		if (Class == nullptr)
		{
			return;
		}

		if ((!Class->HasAllClassFlags(CLASS_NotPlaceable) &&
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
			Class->ImplementsInterface(UStageObject::StaticClass())
			// && Class->ClassGeneratedBy == nullptr // for only c++ class
			)
			|| Force)
		{
			UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("RegisterStageObject:%s"), *Class->GetName());
			UActorFactory* Factory = GEditor->FindActorFactoryByClassForActorClass(UActorFactory::StaticClass(), Class);
			TSharedPtr<FBodyStageObject> Obj = MakeShareable(new FBodyStageObject(Factory, AssetData));
			BodySet* TargetSet = nullptr;

			TargetSet = &BodyObjects[FBodyCategory::GetBodyName()];
			if (TargetSet != nullptr && !CheckAssetDuplicate(*TargetSet, Obj))
			{
				TargetSet->Add(Obj);
			}
		}
	};

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	//Find ALL Accessories(StaticMesh)
	FARFilter MFilter;
	MFilter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
	MFilter.bRecursivePaths = true;
	MFilter.PackagePaths.Add("/Game/Assets/_Art/_Character/_Common/Hair/HairMain");
	TArray< FAssetData > MeshAssetList;
	AssetRegistry.GetAssets(MFilter, MeshAssetList);

	for (FAssetData Asset : MeshAssetList)
	{
		UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("Detect Asset List:%s"), *Asset.PackageName.ToString());
		if (UObject* Obj = Asset.GetAsset()) {
			CheckAssetClass(Obj->StaticClass(), Asset, true);
		}
	}
};

///////////

TSharedRef<ITableRow> SAccessoriesEditorToolbar::OnGenerateWidgetForItem(TSharedPtr<FItemStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	ItemStageObjectAssetEntry = SNew(SItemStageObjectAssetEntry, InItem.ToSharedRef());
	ItemStageObjectAssetEntry->SetSelectItemEventDelegate(TouchItemDelegate);
	ItemStageObjectAssetEntry->SetRemoveDataDelegate(OnDataRemove);
	return SNew(STableRow<TSharedPtr<FItemStageObject>>, OwnerTable)
		[
			ItemStageObjectAssetEntry.ToSharedRef()
			//.HighlightText(this, &SPlacementModeTools::GetHighlightText)
		];
}

TSharedRef<ITableRow> SAccessoriesEditorToolbar::OnGenerateWidgetForBody(TSharedPtr<FBodyStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	BodyStageObjectAssetEntry = SNew(SBodyStageObjectAssetEntry, InItem.ToSharedRef());
	BodyStageObjectAssetEntry->SetSelectBodyEventDelegate(TouchDelegate);
	BodyStageObjectAssetEntry->SetRemoveSingleDataDelegate(OnDataRemove);
	return SNew(STableRow<TSharedPtr<FBodyStageObject>>, OwnerTable)
		[
			BodyStageObjectAssetEntry.ToSharedRef()
			//.HighlightText(this, &SPlacementModeTools::GetHighlightText)
		];
} 

//////////////////////////////


void SAccessoriesEditorToolbar::AddPropertiesRecursive(UProperty* Property)
{
	if (Property != NULL)
	{
		// Add this property
		SelectedObjectProperties.Add(Property);

		// If this is a struct or an array of structs, recursively add the child properties
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
		UStructProperty* StructProperty = Cast<UStructProperty>(Property);
		if (StructProperty != NULL &&
			StructProperty->Struct != NULL)
		{
			for (TFieldIterator<UProperty> StructPropIt(StructProperty->Struct); StructPropIt; ++StructPropIt)
			{
				UProperty* InsideStructProperty = *StructPropIt;
				AddPropertiesRecursive(InsideStructProperty);
			}
		}
		else if (ArrayProperty && ArrayProperty->Inner->IsA<UStructProperty>())
		{
			AddPropertiesRecursive(ArrayProperty->Inner);
		}
	}
}

void SAccessoriesEditorToolbar::UpdateFromObjects(const TArray<UStaticMesh*>& PropertyObjects, struct FSelectionInfo& SelectionInfo, const FShowDetailsOptions& Options)
{
	// There's not an explicit point where
	// we ender a kind of component editing mode, so instead, just look at what we're selecting.
	// If we select a component, then enable the customization.
	
	
	for (auto ObjectIt = PropertyObjects.CreateConstIterator(); ObjectIt; ++ObjectIt)
	{
		SelectionInfo.ObjectsForPropertyEditing.AddUnique(*ObjectIt);
	}	

	
	// Add to the property filter list for any editable component templates
	if (SelectionInfo.EditableComponentTemplates.Num())
	{
		for (auto CompIt = SelectionInfo.EditableComponentTemplates.CreateIterator(); CompIt; ++CompIt)
		{
			UActorComponent* EditableComponentTemplate = *CompIt;
			check(EditableComponentTemplate != NULL);

			// Add all properties belonging to the component template class
			for (TFieldIterator<UProperty> PropIt(EditableComponentTemplate->GetClass()); PropIt; ++PropIt)
			{
				UProperty* Property = *PropIt;
				check(Property != NULL);

				AddPropertiesRecursive(Property);
			}

			// Attempt to locate a matching property for the current component template
			for (auto ObjIt = SelectionInfo.ObjectsForPropertyEditing.CreateIterator(); ObjIt; ++ObjIt)
			{
				UObject* Object = *ObjIt;
				check(Object != NULL);

				if (Object != EditableComponentTemplate)
				{
					UObjectProperty* ObjectProperty = FindField<UObjectProperty>(Object->GetClass(), EditableComponentTemplate->GetFName());
					if (ObjectProperty != nullptr)
					{
						SelectedObjectProperties.Add(ObjectProperty);
					}
					else if (UActorComponent* Archetype = Cast<UActorComponent>(EditableComponentTemplate->GetArchetype()))
					{
						if (AActor* Owner = Archetype->GetOwner())
						{
							if (UClass* OwnerClass = Owner->GetClass())
							{
								AActor* OwnerCDO = CastChecked<AActor>(OwnerClass->GetDefaultObject());
								for (TFieldIterator<UObjectProperty> ObjPropIt(OwnerClass, EFieldIteratorFlags::IncludeSuper); ObjPropIt; ++ObjPropIt)
								{
									ObjectProperty = *ObjPropIt;
									check(ObjectProperty != nullptr);

									// If the property value matches the current archetype, add it as a selected property for filtering
									if (Archetype->GetClass()->IsChildOf(ObjectProperty->PropertyClass)
										&& Archetype == ObjectProperty->GetObjectPropertyValue_InContainer(OwnerCDO))
									{
										ObjectProperty = FindField<UObjectProperty>(Object->GetClass(), ObjectProperty->GetFName());
										if (ObjectProperty != nullptr)
										{
											SelectedObjectProperties.Add(ObjectProperty);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	PropertyViewTitle = Options.ForcedTitle;
	bShowComponents = Options.bShowComponents;

}

FReply SAccessoriesEditorToolbar::OnCleanClicked()
{
	return FReply::Handled();
}

FReply SAccessoriesEditorToolbar::OnExportDataClicked()
{
	InternalExportData();
	return FReply::Handled();
}

FReply SAccessoriesEditorToolbar::OnExportCommonClicked()
{
	InternalExportCommonData();
	return FReply::Handled();
}

FReply SAccessoriesEditorToolbar::OnResetClicked()
{

	return FReply::Handled();
}

void SAccessoriesEditorToolbar::OnMapChanged(UWorld * World, EMapChangeType MapChangeType)
{
	if (!HasRegisterTools)
	{
		bNeedRefreshPlacementContent = true;
		HasRegisterTools = true;
	}
}



void SAccessoriesEditorToolbar::InternalExportData()
{
	FName StaticName = FName(*NewStaticMesh->GetName());
	FName SkeletalName = FName(*NewSkeletalMesh->GetName());
	if (newActorCreated) {
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
		if (EditorChar) {
			EditorChar->SetDataTable(StaticName, SkeletalName, false);
		}
	}
	else {
		for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
		{
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
			if (EditorChar) {
				EditorChar->SetDataTable(StaticName, SkeletalName, false);
			}
		}
	}
	bNeedRefreshBodyContent = true;
}

void SAccessoriesEditorToolbar::InternalExportCommonData()
{
	FName StaticName = FName(*NewStaticMesh->GetName());
	FName SkeletalName = FName(*NewSkeletalMesh->GetName());
	if (newActorCreated) {
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
		if (EditorChar) {
			EditorChar->SetDataTable(StaticName, SkeletalName, true);
		}
	}
	else {
		for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
		{
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
			if (EditorChar) {
				EditorChar->SetDataTable(StaticName, SkeletalName, true);
			}
		}
	}
	bNeedRefreshBodyContent = true;
}

FReply SAccessoriesEditorToolbar::OnSelectMapClicked()
{
	if (bPickMapDialogOpend)
		return FReply::Handled();

	FEditorFileUtils::FOnLevelsChosen LevelsChosenDelegate = FEditorFileUtils::FOnLevelsChosen::CreateSP(this, &SAccessoriesEditorToolbar::OnSelectMap);
	FEditorFileUtils::FOnLevelPickingCancelled LevelPickingCancelledDelegate = FEditorFileUtils::FOnLevelPickingCancelled::CreateSP(this, &SAccessoriesEditorToolbar::OnSelectMapCancel);
	const bool bAllowMultipleSelection = false;
	FEditorFileUtils::OpenLevelPickingDialog(LevelsChosenDelegate, LevelPickingCancelledDelegate, bAllowMultipleSelection);

	return FReply::Handled();
}

void SAccessoriesEditorToolbar::OnSelectMap(const TArray<FAssetData>& SelectedAssets)
{
	bPickMapDialogOpend = false;

	if (SelectedAssets.Num() <= 0)
	{
		return;
	}


	FAssetData ChoosedLevel = SelectedAssets[0];
	MapPackagedName = ChoosedLevel.PackageName.ToString();


	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

	// Add Selected Level To Current World
	ULevelStreaming* Level = EditorLevelUtils::AddLevelToWorld(EditorWorld, *MapPackagedName, AddedLevelStreamingClass);
	EditorLevelUtils::SetStreamingClassForLevel(Level, ULevelStreamingAlwaysLoaded::StaticClass());

	// Change the level's locked status
	if (Level != NULL)
	{
		GEditor->NoteSelectionChange();
		FLevelUtils::ToggleLevelLock(Level->GetLoadedLevel());
	}
}
 
void SAccessoriesEditorToolbar::OnSelectMapCancel()
{
	bPickMapDialogOpend = false;
}


void SAccessoriesEditorToolbar::OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{

}

bool SAccessoriesEditorToolbar::IsToolEnable() const
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	return EditorWorld->GetMapName().Equals("AccessoriesEditorMap");
}

EVisibility SAccessoriesEditorToolbar::GetToolVisibility() const
{
	return IsToolEnable() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SAccessoriesEditorToolbar::SetSelectBodyEvent(bool bNeedRefresh, USkeletalMesh* newSkeleton)
{
	bSkeletalMeshSelected = true;
	
	NewSkeletalMesh = newSkeleton;
	if (newActorCreated) {
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
		if (EditorChar) {
			EditorChar->SetHairComponentMesh(newSkeleton);
		}
	}
	else {
		for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
		{
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
			if (EditorChar) {
				EditorChar->SetHairComponentMesh(newSkeleton);
			}
		}
	}
	/*bNeedRefreshViewport = bNeedRefresh;*/
	UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("SelectBodyEvent Touch"));
	UE_LOG(LogAtelierAccessoriesEditor, Log, TEXT("NewSkeleton = %s"), *NewSkeletalMesh->GetName());

	bNeedRefreshBodyContent = true;
}

void SAccessoriesEditorToolbar::SetSelectItemEvent(bool bNeedRefresh, UStaticMesh* newStatic)
{
	bStaticMeshSelected = true;
	
	NewStaticMesh = newStatic;
	PropertyViewTitle.FromString("Detail");
	if (NewStaticMesh)
	{
		ShowDetailsForSingleObject(NewStaticMesh, FShowDetailsOptions(PropertyViewTitle));
	}
	if (newActorCreated) {
		AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(newActorCreated);
		if (EditorChar) {
			EditorChar->SetAccessoriesComponentMesh(newStatic);
		}
	}
	else{
		for (TActorIterator<AAccessoriesEidtorCharacter> ActorItr(GEditor->GetLevelViewportClients()[0]->GetWorld()); ActorItr; ++ActorItr)
		{
			AAccessoriesEidtorCharacter* EditorChar = Cast<AAccessoriesEidtorCharacter>(*ActorItr);
			if (EditorChar) {
				EditorChar->SetAccessoriesComponentMesh(newStatic);
			}
		}
	}
	/*bNeedRefreshViewport = bNeedRefresh;*/
	UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("SelectItemEvent Touch"));
	UE_LOG(LogAtelierAccessoriesEditor, Log, TEXT("NewStatic = %s"), *NewStaticMesh->GetName());

	bNeedRefreshBodyContent = true;
}



////////////////////////////////////////
/**************************************/
/********       Viewport       ********/
/**************************************/
////////////////////////////////////////

class FBasePoseViewportClient : public FEditorViewportClient
{
public:
	FBasePoseViewportClient(FPreviewScene& InPreviewScene, const TSharedRef<SBasePoseViewport>& InBasePoseViewport)
		: FEditorViewportClient(nullptr, &InPreviewScene, StaticCastSharedRef<SEditorViewport>(InBasePoseViewport))
	{
		SetViewMode(VMI_Lit);
		
		ModeTools->SetWidgetMode(FWidget::WM_Translate);
		Widget->SetUsesEditorModeTools(ModeTools);
		

		// Always composite editor objects after post processing in the editor
		EngineShowFlags.SetCompositeEditorPrimitives(true);
		EngineShowFlags.DisableAdvancedFeatures();

		UpdateLighting();

		// Setup defaults for the common draw helper.
		DrawHelper.bDrawPivot = false;
		DrawHelper.bDrawWorldBox = false;
		DrawHelper.bDrawKillZ = false;
		DrawHelper.bDrawGrid = true;
		DrawHelper.GridColorAxis = FColor(70, 70, 70);
		DrawHelper.GridColorMajor = FColor(40, 40, 40);
		DrawHelper.GridColorMinor = FColor(20, 20, 20);
		DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;

		bStoredShowStats = true;
		bShowStats = true;
		bDisableInput = false;
	}

	/////摄像机不能移动的原因
	/*virtual void Tick(float DeltaTime) override
	{
		FEditorViewportClient::Tick(DeltaTime);
		

	}*/

	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	bool ClickComponent(HActor* ActorHitProxy, const FViewportClick& Click);
	bool ClickActor( AActor* Actor, const FViewportClick& Click, bool bAllowSelectionChange);

	virtual FSceneInterface* GetScene() const override
	{
		return PreviewScene->GetScene();
	}

	virtual FLinearColor GetBackgroundColor() const override
	{
		return FLinearColor::White;
	}

	// End of FEditorViewportClient


	void UpdateLighting()
	{
		const USkeletalMeshEditorSettings* Options = GetDefault<USkeletalMeshEditorSettings>();

		PreviewScene->SetLightDirection(Options->AnimPreviewLightingDirection);
		PreviewScene->SetLightColor(Options->AnimPreviewDirectionalColor);
		PreviewScene->SetLightBrightness(Options->AnimPreviewLightBrightness);
	}
	
private:
	
};

bool FBasePoseViewportClient::ClickComponent(HActor* ActorHitProxy, const FViewportClick& Click) {

	//@todo hotkeys for component placement?

	bool bComponentClicked = false;

	USceneComponent* Component = nullptr;

	if (ActorHitProxy->Actor->IsChildActor())
	{
		AActor* TestActor = ActorHitProxy->Actor;
		do
		{
			Component = TestActor->GetParentComponent();
			TestActor = TestActor->GetParentActor();
		} while (TestActor->IsChildActor());
	}
	else
	{
		UPrimitiveComponent* TestComponent = const_cast<UPrimitiveComponent*>(ActorHitProxy->PrimComponent);
		if (ActorHitProxy->Actor->GetComponents().Contains(TestComponent))
		{
			Component = TestComponent;
		}
	}

	//If the component selected is editor-only, we want to select the non-editor-only component it's attached to
	while (Component != nullptr && Component->IsEditorOnly())
	{
		Component = Component->GetAttachParent();
	}

	if (!ensure(Component != nullptr))
	{
		return false;
	}

	// Pivot snapping
	if (Click.GetKey() == EKeys::MiddleMouseButton && Click.IsAltDown())
	{
		GEditor->SetPivot(GEditor->ClickLocation, true, false);

		return true;
	}
	// Selection only
	else if (Click.GetKey() == EKeys::LeftMouseButton)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ClickingOnComponents", "Clicking on Components"));
		GEditor->GetSelectedComponents()->Modify();

		if (Click.IsControlDown())
		{
			const bool bSelect = !Component->IsSelected();
			if (bSelect)
			{
				UE_LOG(LogAtelier, Log, TEXT("Clicking on Component (CTRL LMB): %s (%s)"), *Component->GetClass()->GetName(), *Component->GetName());
			}
			GEditor->SelectComponent(Component, bSelect, true, true);
			bComponentClicked = true;
		}
		else if (Click.IsShiftDown())
		{
			if (!Component->IsSelected())
			{
				UE_LOG(LogAtelier, Log, TEXT("Clicking on Component (SHIFT LMB): %s (%s)"), *Component->GetClass()->GetName(), *Component->GetName());
				GEditor->SelectComponent(Component, true, true, true);
			}
			bComponentClicked = true;
		}
		else
		{
			GEditor->GetSelectedComponents()->DeselectAll();
			UE_LOG(LogAtelier, Log, TEXT("Clicking on Component (LMB): %s (%s)"), *Component->GetClass()->GetName(), *Component->GetName());
			GEditor->SelectComponent(Component, true, true, true);
			bComponentClicked = true;
		}
	}

	return bComponentClicked;

}

bool FBasePoseViewportClient::ClickActor(AActor* Actor, const FViewportClick& Click, bool bAllowSelectionChange) {
	
	if (Click.GetKey() != EKeys::RightMouseButton)
	{
		if (Click.GetKey() == EKeys::LeftMouseButton && this->Viewport->KeyState(EKeys::T) && Actor)
		{
			TArray<UActorComponent*> Components;
			Actor->GetComponents(Components);
			//SetDebugLightmapSample(&Components, NULL, 0, GEditor->ClickLocation);
		}
		else
			if (Actor)
			{
				if (bAllowSelectionChange)
				{
					const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ClickingOnActors", "Clicking on Actors"));
					GEditor->GetSelectedActors()->Modify();

					// Ctrl- or shift- clicking an actor is the same as regular clicking when components are selected
					const bool bComponentSelected = GEditor->GetSelectedComponentCount() > 0;

					if (Click.IsControlDown() && !bComponentSelected)
					{
						const bool bSelect = !Actor->IsSelected();
						if (bSelect)
						{
							UE_LOG(LogAtelier, Log, TEXT("Clicking on Actor (CTRL LMB): %s (%s)"), *Actor->GetClass()->GetName(), *Actor->GetActorLabel());
						}
						GEditor->SelectActor(Actor, bSelect, true, true);
					}
					else if (Click.IsShiftDown() && !bComponentSelected)
					{
						if (!Actor->IsSelected())
						{
							const bool bSelect = true;
							GEditor->SelectActor(Actor, bSelect, true, true);
						}
					}
					else
					{
						// check to see how many actors need deselecting first - and warn as appropriate
						int32 NumSelectedActors = GEditor->GetSelectedActors()->Num();
						if (NumSelectedActors >= EditorActorSelectionDefs::MaxActorsToSelectBeforeWarning)
						{
							const FText ConfirmText = FText::Format(NSLOCTEXT("UnrealEd", "Warning_ManyActorsToSelectOne", "There are {0} selected actors. Selecting this actor will deselect them all. Are you sure?"), FText::AsNumber(NumSelectedActors));

							FSuppressableWarningDialog::FSetupInfo Info(ConfirmText, NSLOCTEXT("UnrealEd", "Warning_ManyActors", "Warning: Many Actors"), "Warning_ManyActors");
							Info.ConfirmText = NSLOCTEXT("ModalDialogs", "ManyActorsToSelectOneConfirm", "Continue Selection");
							Info.CancelText = NSLOCTEXT("ModalDialogs", "ManyActorsToSelectOneCancel", "Keep Current Selection");

							FSuppressableWarningDialog ManyActorsWarning(Info);
							if (ManyActorsWarning.ShowModal() == FSuppressableWarningDialog::Cancel)
							{
								return false;
							}
						}

						GEditor->SelectNone(false, true, false);
						UE_LOG(LogAtelier, Log, TEXT("Clicking on Actor (LMB): %s (%s)"), *Actor->GetClass()->GetName(), *Actor->GetActorLabel());
						GEditor->SelectActor(Actor, true, true, true);
					}
				}

				return false;
			}
	}

	return false;

}


void FBasePoseViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) {

	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
	if (!ModeTools->HandleClick(this, HitProxy, Click))
	{
		if (HitProxy->IsA(HActor::StaticGetType()))
		{
			HActor* ActorHitProxy = (HActor*)HitProxy;
			AActor* ConsideredActor = ActorHitProxy->Actor;
			while (ConsideredActor->IsChildActor())
			{
				ConsideredActor = ConsideredActor->GetParentActor();
			}

			// We want to process the click on the component only if:
			// 1. The actor clicked is already selected
			// 2. The actor selected is the only actor selected
			// 3. The actor selected is blueprintable
			// 4. No components are already selected and the click was a double click
			// 5. OR, a component is already selected and the click was NOT a double click
			const bool bActorAlreadySelectedExclusively = GEditor->GetSelectedActors()->IsSelected(ConsideredActor) && (GEditor->GetSelectedActorCount() == 1);
			const bool bActorIsBlueprintable = FKismetEditorUtilities::CanCreateBlueprintOfClass(ConsideredActor->GetClass());
			const bool bComponentAlreadySelected = GEditor->GetSelectedComponentCount() > 0;
			const bool bWasDoubleClick = (Click.GetEvent() == IE_DoubleClick);
			
			const bool bSelectComponent = bActorAlreadySelectedExclusively && bActorIsBlueprintable && (bComponentAlreadySelected != bWasDoubleClick);

			if (bSelectComponent)
			{
				ClickComponent(ActorHitProxy, Click);
				
			}
			else
			{
				ClickActor(ConsideredActor, Click, true);
			}

			// We clicked an actor, allow the pivot to reposition itself.
			// GUnrealEd->SetPivotMovedIndependently(false);
		}
	}

}



/*-----------------------------------------------------------------------------
EditorViewportToolBar
-----------------------------------------------------------------------------*/

class EditorViewportToolBar : public SViewportToolBar  
{
public:
	SLATE_BEGIN_ARGS(EditorViewportToolBar) {}
	SLATE_ARGUMENT(TWeakPtr<SBasePoseViewport>, EditorViewport)
	SLATE_END_ARGS()

		/** Constructs this widget with the given parameters */
		void Construct(const FArguments& InArgs)
	{
		EditorViewport = InArgs._EditorViewport;

		static const FName DefaultForegroundName("DefaultForeground");



		TSharedRef<STransformViewportToolBar> TransformViewportToolBar = SNew(STransformViewportToolBar);


		this->ChildSlot
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.ColorAndOpacity(this, &SViewportToolBar::OnGetColorAndOpacity)
			.ForegroundColor(FEditorStyle::GetSlateColor(DefaultForegroundName))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 2.0f)
			[
				SNew(SEditorViewportToolbarMenu)
				.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Image("EditorViewportToolBar.MenuDropdown")
			.OnGetMenuContent(this, &EditorViewportToolBar::GeneratePreviewMenu)
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 2.0f)
			[
				SNew(SEditorViewportToolbarMenu)
				.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Label(this, &EditorViewportToolBar::GetCameraMenuLabel)
			.LabelIcon(this, &EditorViewportToolBar::GetCameraMenuLabelIcon)
			.OnGetMenuContent(this, &EditorViewportToolBar::GenerateCameraMenu)
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 2.0f)
			[
				SNew(SEditorViewportToolbarMenu)
				.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Label(this, &EditorViewportToolBar::GetViewMenuLabel)
			.LabelIcon(this, &EditorViewportToolBar::GetViewMenuLabelIcon)
			.OnGetMenuContent(this, &EditorViewportToolBar::GenerateViewMenu)
			]
		+ SHorizontalBox::Slot()
			.Padding(3.0f, 1.0f)
			.HAlign(HAlign_Right)
			[
				//123123
				SNew(STransformViewportToolBar)
				.Viewport(EditorViewport.Pin().ToSharedRef())
				.CommandList(EditorViewport.Pin()->GetCommandList())
			
			]
			]
			];

		SViewportToolBar::Construct(SViewportToolBar::FArguments());
	}

	/** Creates the preview menu */
	TSharedRef<SWidget> GeneratePreviewMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid() ? EditorViewport.Pin()->GetCommandList() : NULL;

		const bool bInShouldCloseWindowAfterMenuSelection = true;

		FMenuBuilder PreviewOptionsMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);
		{
			PreviewOptionsMenuBuilder.BeginSection("BlueprintEditorPreviewOptions", NSLOCTEXT("BlueprintEditor", "PreviewOptionsMenuHeader", "Preview Viewport Options"));
			{
				/*PreviewOptionsMenuBuilder.AddMenuEntry(FBlueprintEditorCommands::Get().ResetCamera);
				PreviewOptionsMenuBuilder.AddMenuEntry(FBlueprintEditorCommands::Get().ShowFloor);
				PreviewOptionsMenuBuilder.AddMenuEntry(FBlueprintEditorCommands::Get().ShowGrid);*/
				PreviewOptionsMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleRealTime);
			}
			PreviewOptionsMenuBuilder.EndSection();
		}

		return PreviewOptionsMenuBuilder.MakeWidget();
	}

	FText GetCameraMenuLabel() const
	{
		FText Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Default", "Camera");

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewportType())
			{
			case LVT_Perspective:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Perspective", "Perspective");
				break;

			case LVT_OrthoXY:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Top", "Top");
				break;

			case LVT_OrthoYZ:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Left", "Left");
				break;

			case LVT_OrthoXZ:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Front", "Front");
				break;

			case LVT_OrthoNegativeXY:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Bottom", "Bottom");
				break;

			case LVT_OrthoNegativeYZ:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Right", "Right");
				break;

			case LVT_OrthoNegativeXZ:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Back", "Back");
				break;

			case LVT_OrthoFreelook:
				Label = NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_OrthoFreelook", "Ortho");
				break;
			}
		}

		return Label;
	}

	const FSlateBrush* GetCameraMenuLabelIcon() const
	{
		static FName PerspectiveIconName("EditorViewport.Perspective");
		static FName TopIconName("EditorViewport.Top");
		static FName LeftIconName("EditorViewport.Left");
		static FName FrontIconName("EditorViewport.Front");
		static FName BottomIconName("EditorViewport.Bottom");
		static FName RightIconName("EditorViewport.Right");
		static FName BackIconName("EditorViewport.Back");

		FName Icon = NAME_None;

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewportType())
			{
			case LVT_Perspective:
				Icon = PerspectiveIconName;
				break;

			case LVT_OrthoXY:
				Icon = TopIconName;
				break;

			case LVT_OrthoYZ:
				Icon = LeftIconName;
				break;

			case LVT_OrthoXZ:
				Icon = FrontIconName;
				break;

			case LVT_OrthoNegativeXY:
				Icon = BottomIconName;
				break;

			case LVT_OrthoNegativeYZ:
				Icon = RightIconName;
				break;

			case LVT_OrthoNegativeXZ:
				Icon = BackIconName;
				break;
			}
		}

		return FEditorStyle::GetBrush(Icon);
	}

	TSharedRef<SWidget> GenerateCameraMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid() ? EditorViewport.Pin()->GetCommandList() : nullptr;

		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder CameraMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);

		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Perspective);

		CameraMenuBuilder.BeginSection("LevelViewportCameraType_Ortho", NSLOCTEXT("BlueprintEditor", "CameraTypeHeader_Ortho", "Orthographic"));
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
		CameraMenuBuilder.EndSection();

		return CameraMenuBuilder.MakeWidget();
	}

	FText GetViewMenuLabel() const
	{
		FText Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Default", "View");

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewMode())
			{
			case VMI_Lit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Lit", "Lit");
				break;

			case VMI_Unlit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Unlit", "Unlit");
				break;

			case VMI_BrushWireframe:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Wireframe", "Wireframe");
				break;
			}
		}

		return Label;
	}

	const FSlateBrush* GetViewMenuLabelIcon() const
	{
		static FName LitModeIconName("EditorViewport.LitMode");
		static FName UnlitModeIconName("EditorViewport.UnlitMode");
		static FName WireframeModeIconName("EditorViewport.WireframeMode");

		FName Icon = NAME_None;

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewMode())
			{
			case VMI_Lit:
				Icon = LitModeIconName;
				break;

			case VMI_Unlit:
				Icon = UnlitModeIconName;
				break;

			case VMI_BrushWireframe:
				Icon = WireframeModeIconName;
				break;
			}
		}

		return FEditorStyle::GetBrush(Icon);
	}

	TSharedRef<SWidget> GenerateViewMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid() ? EditorViewport.Pin()->GetCommandList() : nullptr;

		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder ViewMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);

		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().LitMode, NAME_None, NSLOCTEXT("BlueprintEditor", "LitModeMenuOption", "Lit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().UnlitMode, NAME_None, NSLOCTEXT("BlueprintEditor", "UnlitModeMenuOption", "Unlit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().WireframeMode, NAME_None, NSLOCTEXT("BlueprintEditor", "WireframeModeMenuOption", "Wireframe"));

		return ViewMenuBuilder.MakeWidget();
	}

private:
	/** Reference to the parent viewport */
	TWeakPtr<SBasePoseViewport> EditorViewport;
};

////////////////////////////////
// SBasePoseViewport
void SBasePoseViewport::Construct(const FArguments& InArgs)
{
	SEditorViewport::Construct(SEditorViewport::FArguments());

	

	USkeletalMesh* HeadMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Assets/_Art/_Character/_Common/Facial/Head/Ciruru/Ciruru_morpher_BKF.Ciruru_morpher_BKF'"));
	USkeletalMesh* BodyMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Assets/_Art/_Character/Heroine/Suit/PlayerSkin_JK/Model/PlayerSkin_JK_BKF.PlayerSkin_JK_BKF'"));
	
	PreviewComponent = NewObject<USkeletalMeshComponent>();
	MeshComponent = NewObject<UStaticMeshComponent>();
	HeadComponent = NewObject<USkeletalMeshComponent>();
	BodyComponent = NewObject<USkeletalMeshComponent>();

	HeadComponent->SetSkeletalMesh(HeadMesh);
	BodyComponent->SetSkeletalMesh(BodyMesh);
	PreviewScene.SetSkyBrightness(10);
	//PreviewComponent->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	
	Actor = PreviewScene.GetWorld()->SpawnActor<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(),FVector(),FRotator());
	Actor->SetActorLabel("TestMyViewPort");
	Actor->GetSkeletalMeshComponent()->SetSkeletalMesh(BodyMesh);
	HeadComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	PreviewComponent->AttachToComponent(HeadComponent, FAttachmentTransformRules::KeepRelativeTransform, "HairMain");
	MeshComponent->AttachToComponent(PreviewComponent, FAttachmentTransformRules::KeepRelativeTransform, "Point_Hair_Root");
	HeadComponent->RegisterComponentWithWorld(PreviewScene.GetWorld());
	PreviewComponent->RegisterComponentWithWorld(PreviewScene.GetWorld());
	MeshComponent->RegisterComponentWithWorld(PreviewScene.GetWorld());
	Actor->RegisterAllComponents();

	FVector NewPosition = HeadComponent->GetSocketLocation(FName("HairMain")) + FVector(0.f, 50.f, 0.f);
	Client->SetViewLocation(NewPosition);
	
}


void SBasePoseViewport::SetSkeleton(USkeletalMesh* Skeleton)
{
	if (Skeleton != TargetSkeleton)
	{
		TargetSkeleton = Skeleton;

		if (TargetSkeleton)
		{
			USkeletalMesh* PreviewSkeletalMesh = Skeleton;
			if (PreviewSkeletalMesh)
			{
				PreviewComponent->SetSkeletalMesh(PreviewSkeletalMesh);
				////Place the camera at a good viewer position
				FVector NewPosition = HeadComponent->GetSocketLocation(FName("HairMain")) + FVector(0.f, 50.f, 0.f);
				Client->SetViewLocation(NewPosition);
			} 
			else
			{
				PreviewComponent->SetSkeletalMesh(nullptr); 
			}
		}
		else
		{
			PreviewComponent->SetSkeletalMesh(nullptr);
		}
		Client->Invalidate();
	}
}

void SBasePoseViewport::SetStatic(UStaticMesh* Static)
{
	if (Static != TargetStatic)
	{
		TargetStatic = Static;

		if (TargetStatic)
		{
			UStaticMesh* PreviewStaticMesh = Static;
			if (PreviewStaticMesh)
			{
				Client->Invalidate(true,false);
				UE_LOG(LogAtelierAccessoriesEditor, Warning, TEXT("PreviewStaticMesh is Set"));

				MeshComponent->SetStaticMesh(PreviewStaticMesh);
				MeshComponent->bSelectable = true;
				GEditor->SelectNone(false, true, false);
				GEditor->SelectActor(Actor, true, true, true);
				Client->SetWidgetMode(FWidget::WM_Translate);
				Client->Invalidate(true, false);
				
			}
			else
			{
				MeshComponent->SetStaticMesh(nullptr);
			}
		}
		else
		{
			MeshComponent->SetStaticMesh(nullptr);
		}
	}
}

SBasePoseViewport::SBasePoseViewport()
	: PreviewScene(FPreviewScene::ConstructionValues())
{
}

bool SBasePoseViewport::IsVisible() const
{
	return true;
}

TSharedRef<FEditorViewportClient> SBasePoseViewport::MakeEditorViewportClient()
{
	TSharedPtr<FEditorViewportClient> EditorViewportClient = MakeShareable(new FBasePoseViewportClient(PreviewScene, SharedThis(this)));

	EditorViewportClient->ViewportType = LVT_Perspective;
	EditorViewportClient->bSetListenerPosition = false;
	EditorViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
	EditorViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);
	EditorViewportClient->SetCameraSpeedSetting(1);
	EditorViewportClient->SetRequiredCursorOverride(true, EMouseCursor::Crosshairs);
	EditorViewportClient->SetRealtime(true);
	EditorViewportClient->bDrawAxes = true;
	EditorViewportClient->VisibilityDelegate.BindSP(this, &SBasePoseViewport::IsVisible);
	EditorViewportClient->SetViewMode(VMI_Lit);

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SBasePoseViewport::MakeViewportToolbar()
{
	return
		SNew(EditorViewportToolBar)
		.EditorViewport(SharedThis(this))
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());

}

#undef LOCTEXT_NAMESPACE
////////////////////////////////////////
