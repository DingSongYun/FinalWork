// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YU
// Date: 2019-02-19

#include "SAvatarEditorToolbar.h"
#include "SButton.h"
#include "Editor.h"
#include "UObject/UnrealType.h"
#include "UnrealEdGlobals.h"
#include "EngineGlobals.h"
#include "ConstructorHelpers.h"
#include "SlateTypes.h"
#include "SBoxPanel.h"
#include "EditorStyle.h"
#include "EngineUtils.h"
#include "EditorUtilities.h"
#include "EditorLevelUtils.h"
#include "LevelEditor.h"
#include "Settings/LevelEditorMiscSettings.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "EditorModeManager.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "JsonObjectConverter.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "PaperSpriteBlueprintLibrary.h"
#include "DataTables/AvatarTransData.h"

#define LOCTEXT_NAMESPACE "AvatarStructCustomization"
DEFINE_LOG_CATEGORY(LogAvatarEditor)

SAvatarEditorToolbar::SAvatarEditorToolbar()
{
	InitToolState();

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	OnMapChangedHandler = LevelEditorModule.OnMapChanged().AddRaw(this, &SAvatarEditorToolbar::OnMapChanged);
	RegisterItemObject();
}

SAvatarEditorToolbar::~SAvatarEditorToolbar()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnMapChanged().Remove(OnMapChangedHandler);
}


void SAvatarEditorToolbar::Construct(const FArguments & InArgs)
{
	FEditorUtilities::OpenLevelByName("AvatarEditorMap");
	//ShowWidgetInEditor();

	TouchItemDelegate.BindSP(this, &SAvatarEditorToolbar::SetupEditorChar);
	OnDataRemove.BindSP(this, &SAvatarEditorToolbar::RefreshContentEvent);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateEditorToolsPanel()
			]
			+ SVerticalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(HAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0, 5, 0, 0)
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
				.Value(1.0f)
				[
					CreateItemPanel()
				]
			]
		];
}


void SAvatarEditorToolbar::Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bNeedRefreshContent) {
		RefreshContent();
	}
}

void SAvatarEditorToolbar::InitToolState()
{
	HasRegisterTools = false;
}

void SAvatarEditorToolbar::OnMapChanged(UWorld * World, EMapChangeType MapChangeType)
{
	InitEditorTool();
	if (!HasRegisterTools)
	{
		HasRegisterTools = true;
	}

}

void SAvatarEditorToolbar::ShowWidgetInEditor()
{
	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TWeakPtr<SLevelEditor> editor = LevelEditor.GetLevelEditorInstance();
	TSharedPtr<SLevelEditor> PinnedEditor(editor.Pin());


	if (PinnedEditor.IsValid())
	{
		TSharedPtr<SLevelViewport> slevelviewport = StaticCastSharedPtr<SLevelViewport>(LevelEditor.GetFirstActiveViewport());
		TWeakPtr<SViewport> viewportWidget = slevelviewport->GetViewportWidget();

		FChildren* childrenSlot = slevelviewport->GetChildren();
		if (childrenSlot)
		{
			const TSharedPtr<SWidget>& Child = viewportWidget.Pin()->GetContent();
			TSharedPtr<SOverlay> ViewportWidget = StaticCastSharedPtr<SOverlay>(Child);

			ViewportWidget->AddSlot()[
				SNew(SOverlay)
					+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Left)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(GetAvatarFrameBrush())
							]
							+ SOverlay::Slot()
								.Padding(11.1, 3.1, 11.2, 19.299999)
							[
								SNew(SCanvas)
								+ SCanvas::Slot()
								.Size(FVector2D(512,512))
								.Position(FVector2D(0,0))
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SBorder)
									.Clipping(EWidgetClipping::ClipToBounds)
									.VAlign(VAlign_Fill)
									.HAlign(HAlign_Fill)
									[
										SNew(SCanvas)
										+ SCanvas::Slot()
										.Size(FVector2D(0, 0))
										.Position(FVector2D(0, 0))
										.VAlign(VAlign_Center)
										.HAlign(HAlign_Center)
										[
											SNew(SImage)
											.Image(GetAvatarBrush())
										]
										
									]
									
								]
							]
						]
					
			];
		}
	}
}



const FSlateBrush* SAvatarEditorToolbar::GetAvatarFrameBrush()
{
	const FString IconPath = "UPaperSprite '/Game/Assets/_Art/_UI/_Texture/_NewMainMenu/Frames/avatar_bottom_1_png.avatar_bottom_1_png'";
	UPaperSprite* AvatarFrameImg = LoadObject<UPaperSprite>(NULL, *IconPath);
	
	FSlateBrush AvatarFrame;
	FSlateBrush* brush = new FSlateBrush(AvatarFrame);
	if (AvatarFrameImg == nullptr) {
		return brush;
	}
	const FSlateAtlasData SpriteAtlasData = AvatarFrameImg->GetSlateAtlasData();
	const FVector2D SpriteSize = SpriteAtlasData.GetSourceDimensions();
	brush->SetResourceObject(AvatarFrameImg);
	brush->ImageSize.X = SpriteSize.X;
	brush->ImageSize.Y = SpriteSize.Y;
	brush->DrawAs = ESlateBrushDrawType::Image;

	return brush;
}
 
const FSlateBrush * SAvatarEditorToolbar::GetAvatarBrush()
{
	const FString Path = "UMaterialInterface '/Game/Tools/AvatarEditorTool/AvatarEditorRT_Mat.AvatarEditorRT_Mat'";
	UMaterialInterface* AvatarMat = LoadObject<UMaterialInterface>(NULL, *Path);
	FSlateBrush AvatarBrush;
	FSlateBrush* brush = new FSlateBrush(AvatarBrush);
	UE_LOG(LogTemp, Warning, TEXT("finding AvatarMat"));
	if (AvatarMat == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to find AvatarMat"));
		return brush;
	}
	brush->SetResourceObject(AvatarMat);
	brush->ImageSize.X = 512;
	brush->ImageSize.Y = 512;
	brush->DrawAs = ESlateBrushDrawType::Image;

	return brush;
}

TSharedRef<SBorder> SAvatarEditorToolbar::CreateItemPanel()
{
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	TSharedRef<SBorder> PlaceStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0)
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
								SAssignNew(NpcListView, SListView<TSharedPtr<FNpcStageObject>>)
								.ListItemsSource(&FilteredItems)
								.OnGenerateRow(this, &SAvatarEditorToolbar::OnGenerateWidgetForItem)
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
		];
	return PlaceStageObjectPanel;
}

TSharedRef<SWrapBox> SAvatarEditorToolbar::CreateEditorToolsPanel()
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
				// Button SaveData
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SAvatarEditorToolbar::OnSaveDataClicked)
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
							.Text(FEditorFontGlyphs::Pencil_Square_O)
							]
						// Import Text
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Text(FText::FromString("SaveData"))
							]
					]
				]

				//Button SelectComp
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SAvatarEditorToolbar::OnSelectCompClicked)
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
							.Text(FText::FromString("SelectMeshComp"))
							]
					]
				]
			]
		];
			

	return EditorToolPanel;
}

FReply SAvatarEditorToolbar::OnSaveDataClicked()
{
	if (EditorTool != nullptr) {
		//EditorTool->SaveData();
	}
	bNeedRefreshContent = true;
	return FReply::Handled();
}

FReply SAvatarEditorToolbar::OnSelectCompClicked()
{
	this->SelectCompEvent();
	return FReply::Handled();
}

void SAvatarEditorToolbar::RefreshContentEvent()
{
	bNeedRefreshContent = true;
}

void SAvatarEditorToolbar::SelectCompEvent()
{
	EditorTool->SelectComp();
}

void SAvatarEditorToolbar::RegisterItemObject()
{
	UGADataTable* npcdata = UGADataTable::GetDataTable("NPC");
	TArray<UGADataRow*> rows;
	npcdata->GetAllRows(rows);

	for (int32 i = 0; i < rows.Num(); i++) {
		UGADataRow* row = rows[i];
		int32 d_npcid = row->GetNumber("ID");
		if (d_npcid != 100101) {
			TSharedPtr<FNpcStageObject> Obj = MakeShareable(new FNpcStageObject(d_npcid));
			NPCItems* TargetSet = nullptr;
			TargetSet = &FilteredItems;
			TargetSet->Add(Obj);
		}
	}

}

TSharedRef<ITableRow> SAvatarEditorToolbar::OnGenerateWidgetForItem(TSharedPtr<FNpcStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	NpcStageObjectAssetEntry = SNew(SNpcStageObjectAssetEntry, InItem.ToSharedRef());
	NpcStageObjectAssetEntry->SetSelectItemEventDelegate(TouchItemDelegate);
	NpcStageObjectAssetEntry->SetRemoveDataEventDelegate(OnDataRemove);
	return SNew(STableRow<TSharedPtr<FNpcStageObject>>, OwnerTable)
		[
			NpcStageObjectAssetEntry.ToSharedRef()
		];
}

void SAvatarEditorToolbar::InitEditorTool()
{
	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
	for (TActorIterator<AAvatarEditorTool> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		EditorTool = Cast<AAvatarEditorTool>(*ActorItr);
	}
	if (EditorTool != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Find Editor Tool"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find Editor Tool"));
	} 
	
}

void SAvatarEditorToolbar::SetupEditorChar(TSharedPtr<const FNpcStageObject> item)
{
	int32 npcid = item->Npcid;
	if (EditorTool != nullptr)
	{
		/*EditorTool->CurrNpcID = item->Npcid;
		EditorTool->LoadData();
		EditorTool->SetupEditorChar(npcid);
		EditorTool->SelectComp();*/
	}
}

void SAvatarEditorToolbar::RefreshContent()
{
	if (!bNeedRefreshContent) {
		return;
	}

	bNeedRefreshContent = false;
	NpcListView->RebuildList();
}

void SNpcStageObjectAssetEntry::Construct(const FArguments & InArgs, const TSharedPtr<const FNpcStageObject>& InItem)
{
	bIsPressed = false;
	Item = InItem;
	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");
	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;
	ChildSlot
		[
			SNew(SBorder)
			.Cursor(EMouseCursor::Hand)
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
				.AutoWidth()
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
						.Padding(0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(this->CheckStyle(Item->Npcid))
							.Visibility(this->IsSetCurrentData(Item->Npcid))
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

FReply SNpcStageObjectAssetEntry::OnMouseButtonDown(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectItemEventDelegate.ExecuteIfBound(Item);
	}
	FText Style = this->CheckStyle(Item->Npcid);
	EVisibility Visible = this->IsSetCurrentData(Item->Npcid);
	UE_LOG(LogTemp, Warning, TEXT("Visible = %s"), *Visible.ToString());
	return FReply::Unhandled();
}

void SNpcStageObjectAssetEntry::SetSelectItemEventDelegate(FSelectNpcItemEvent SelectDelegate)
{
	SelectItemEventDelegate = SelectDelegate;
}

void SNpcStageObjectAssetEntry::SetRemoveDataEventDelegate(FRemoveDataEvent RemoveDelegate)
{
	RemoveDataEventDelegate = RemoveDelegate;
}

FText SNpcStageObjectAssetEntry::CheckStyle(int32 npcid)
{
	if (!npcid) {
		return FEditorFontGlyphs::Check_Square_O;
	}

	UDataTable* DataTable;
	FString Context;
	FAvatarTransData row;
	FTransform LoadTransform;
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/AvatarTransData.AvatarTransData'"));
	if (DataTable->GetRowNames().Num())
	{
		TArray<FName> RowNames = DataTable->GetRowNames();
		if (RowNames.Contains(FName(*FString::FromInt(npcid)))) {
			return FEditorFontGlyphs::Check_Square;
		}
		else
		{
			return FEditorFontGlyphs::Check_Square_O;
		}
	}

	return FEditorFontGlyphs::Check_Square_O;
}

EVisibility SNpcStageObjectAssetEntry::IsSetCurrentData(int32 npcid)
{
	if (!npcid) {
		return EVisibility::Hidden;
	}

	UDataTable* DataTable;
	FString Context;
	FAvatarTransData row;
	FTransform LoadTransform;
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/AvatarTransData.AvatarTransData'"));
	if (DataTable->GetRowNames().Num())
	{
		TArray<FName> RowNames = DataTable->GetRowNames();
		if (RowNames.Contains(FName(*FString::FromInt(npcid)))) {
			return EVisibility::Visible;
		}
	}
	return EVisibility::Hidden;
}

TSharedRef<SWrapBox> SNpcStageObjectAssetEntry::CreateRemoveDataPanel()
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
					.OnClicked(this, &SNpcStageObjectAssetEntry::OnRemoveDataClicked)
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

FReply SNpcStageObjectAssetEntry::OnRemoveDataClicked()
{
	FString PackagePath = FString("/Game/Data/DataTables/AvatarTransData");
	UPackage *Package = CreatePackage(nullptr, *PackagePath);
	UDataTable* DataTable;
	FName ItemName = FName(*FString::FromInt(Item->Npcid));
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/AvatarTransData.AvatarTransData'"));
	if (DataTable != nullptr)
	{
		TArray<FAvatarTransData*> OutAllRows;
		FString Context; // Used in error reporting.
		DataTable->GetAllRows<FAvatarTransData>(Context, OutAllRows); // Populates the array with all the data from the CSV.
		TArray<FName> key = DataTable->GetRowNames();
		if (key.Contains(ItemName)) {
			DataTable->RemoveRow(ItemName);
			FString AssetPath = FString("D:/Game/atelier/client-ue4/Content/Data/DataTables/");
			FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("AvatarTransData"), *FPackageName::GetAssetPackageExtension());
			UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
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
	}
	RemoveDataEventDelegate.ExecuteIfBound();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
