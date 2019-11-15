// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#include "SMapEditorToolbar.h"
#include "Editor.h"
#include "SImage.h"
#include "SButton.h"
#include "SBoxPanel.h"
#include "SSplitter.h"
#include "SListView.h"
#include "STextBlock.h"
#include "LevelUtils.h"
#include "EditorStyle.h"
#include "Editor/EditorEngine.h"
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
#include "StageObjectSaveHandler.h"
#include "AssetRegistryModule.h"
#include "AssetSelection.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Utilities/Utilities.h"
#include "MapEditorTool.h"
#include "EditorPlayerSettings.h"

DEFINE_LOG_CATEGORY(LogAtelierMapEditor)
const FString MAP_EDITOR_SCENE_NAME = "MapEditor";

/*****************************************************************/
/***************SPlacementStageObjectAssetEntry*******************/
/*****************************************************************/
void SPlacementStageObjectAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FPlacementStageObject>& InItem)
{
	bIsPressed = false;

	Item = InItem;

	TSharedPtr< SHorizontalBox > ActorType = SNew( SHorizontalBox );

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
	if(DefaultActor != nullptr)
	{
		DocClass = DefaultActor->GetClass();
		AssetEntryToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
	}

	if (!AssetEntryToolTip.IsValid())
	{
		AssetEntryToolTip = FSlateApplicationBase::Get().MakeToolTip(Item->DisplayName);
	}
	
	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>( "PlacementBrowser.Asset" );

	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed; 

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( this, &SPlacementStageObjectAssetEntry::GetBorder )
		.Cursor( EMouseCursor::GrabHand )
		.ToolTip( AssetEntryToolTip )
		[
			SNew( SHorizontalBox )

			+ SHorizontalBox::Slot()
			.Padding( 0 )
			.AutoWidth()
			[
				// Drop shadow border
				SNew( SBorder )
				.Padding( 4 )
				.BorderImage( FEditorStyle::GetBrush( "ContentBrowser.ThumbnailShadow" ) )
				[
					SNew( SBox )
					.WidthOverride( 35 )
					.HeightOverride( 35 )
					//[
					//	SNew( SPlacementAssetThumbnail, Item->AssetData )
					//	.ClassThumbnailBrushOverride( Item->ClassThumbnailBrushOverride )
					//	.AlwaysUseGenericThumbnail( Item->bAlwaysUseGenericThumbnail )
					//	.AssetTypeColorOverride( Item->AssetTypeColorOverride )
					//]
				]
			]

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 4, 0)
			[
				SNew( SVerticalBox )
				+SVerticalBox::Slot()
				.Padding(0, 0, 0, 1)
				.AutoHeight()
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "PlacementBrowser.Asset.Name" )
					.Text( Item->DisplayName )
					.HighlightText(InArgs._HighlightText)
				]
			]
		]
	];
}

const FSlateBrush* SPlacementStageObjectAssetEntry::GetBorder() const
{
	if ( IsPressed() )
	{
		return PressedImage;
	}
	else if ( IsHovered() )
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}

bool SPlacementStageObjectAssetEntry::IsPressed() const
{
	return bIsPressed;
}

FReply SPlacementStageObjectAssetEntry::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	if (FEditorDelegates::OnAssetDragStarted.IsBound())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add( Item->AssetData );
		FEditorDelegates::OnAssetDragStarted.Broadcast( DraggedAssetDatas, Item->Factory );
		return FReply::Handled();
	}

	if( MouseEvent.IsMouseButtonDown( EKeys::LeftMouseButton ) )
	{
		return FReply::Handled().BeginDragDrop( FAssetDragDropOp::New( Item->AssetData, Item->Factory ) );
	}
	else
	{
		return FReply::Handled();
	}
}

FReply SPlacementStageObjectAssetEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
	{
		bIsPressed = true;

		return FReply::Handled().DetectDrag( SharedThis( this ), MouseEvent.GetEffectingButton() );
	}

	return FReply::Unhandled();
}

FReply SPlacementStageObjectAssetEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
	{
		bIsPressed = false;
	}

	return FReply::Unhandled();
}


/*****************************************************************/
/************************SMapEditorToolbar************************/
/*****************************************************************/
SMapEditorToolbar::SMapEditorToolbar()
	:AddedLevelStreamingClass(ULevelStreamingDynamic::StaticClass())
{
	const TSubclassOf<ULevelStreaming> DefaultLevelStreamingClass = GetDefault<ULevelEditorMiscSettings>()->DefaultLevelStreamingClass;
	if ( DefaultLevelStreamingClass )
	{
		AddedLevelStreamingClass = DefaultLevelStreamingClass;
	} 

	RegisterPlaceableStageObjec();
	InitToolState();

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	OnMapChangedHandler = LevelEditorModule.OnMapChanged().AddRaw(this, &SMapEditorToolbar::OnMapChanged);
	EditorTool = nullptr;
}

SMapEditorToolbar::~SMapEditorToolbar()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnMapChanged().Remove(OnMapChangedHandler);
	CleanAll();
}


void SMapEditorToolbar::InitToolState()
{
	bPickMapDialogOpend = false;
	MapPackagedName = "";
	ActivePlacementCategory = "";
	HasRegisterTools = false;
}

void SMapEditorToolbar::Construct(const FArguments& InArgs)
{
	//RegisterPlaceableStageObjec();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(OpenEditorSceneButton, SButton)
			.Text(FText::FromString("Open Map Editor Scene"))
			.OnClicked( this, &SMapEditorToolbar::OnOpenMapEditorSceneClicked)
			.Visibility(GetToolVisibility() == EVisibility::Visible ? EVisibility::Collapsed : EVisibility::Visible)
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0)
		[
			SAssignNew(MainToolPanel, SVerticalBox)
			.Visibility(GetToolVisibility())

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateToolbarPanel()
			]

			+ SVerticalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0,5,0,0)
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
				.Value(0.5f)
				[
					CreatePlaceStageObjectPanel()
				]
				+ SSplitter::Slot()
				.Value(0.5f)
				[
					CreateEditorToolsPanel()
				]
			]
		]
	];
}

UWorld* SMapEditorToolbar::GetWorld()
{
	return  GEditor->GetEditorWorldContext().World();
}

void SMapEditorToolbar::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (bNeedRefreshPlacementContent)
	{
		RefreshPlacementContent();
	}
}

TSharedRef<SWrapBox> SMapEditorToolbar::CreateToolbarPanel()
{
	TSharedRef<SWrapBox> ToolBarPanel = SNew(SWrapBox)
		.UseAllottedWidth( true )
		.InnerSlotPadding( FVector2D( 5, 2 ) );

	ToolBarPanel->AddSlot()
		.FillLineWhenWidthLessThan( 600 )
		.FillEmptySpace( false )
		[
			SNew( SBorder )
			.Padding( FMargin( 3 ) )
			.BorderImage( FEditorStyle::GetBrush( "ToolPanel.GroupBorder" ) )
			[
				SNew( SHorizontalBox )

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign( VAlign_Center )
				.HAlign( HAlign_Left )
				[
					SNew( SButton )

					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked( this, &SMapEditorToolbar::OnCleanMapClicked )
					.ContentPadding(FMargin(6, 2))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
						SNew( SHorizontalBox )

						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Ship)
						]

						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew( STextBlock )
							.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
							.Text( FText::FromString( "Clean Map" ) )
						]
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign( VAlign_Center )
				.HAlign( HAlign_Left )
				.Padding(20, 0, 0, 0)
				[
					SNew( SButton )

					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked( this, &SMapEditorToolbar::OnCleanMapConfigObjectsClicked)
					.ContentPadding(FMargin(6, 2))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
						SNew( SHorizontalBox )

						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Ship)
						]

						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew( STextBlock )
							.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
							.Text( FText::FromString( "Clean Config Objects" ) )
						]
					]
				]
			]
		];
	return ToolBarPanel;
}

TSharedRef<SBorder> SMapEditorToolbar::CreatePlaceStageObjectPanel()
{
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	TSharedRef<SVerticalBox> Tabs = SNew(SVerticalBox);
	for (auto& Element : PlaceableObjects)
	{
		Tabs->AddSlot()
		.AutoHeight()
		[
			CreatePlacementGroupTab(PlacementTabInfo(Element.Key))
		];
	}


	TSharedRef<SBorder> PlaceStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew( SVerticalBox )

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew( SearchBoxPtr, SSearchBox )
				.HintText(NSLOCTEXT("Map Editor", "SearchPlaceables", "Search Stage Object"))
				.OnTextChanged(this, &SMapEditorToolbar::OnSearchChanged)
				.OnTextCommitted(this, &SMapEditorToolbar::OnSearchCommitted)
			]

			+ SVerticalBox::Slot()
			.Padding( 0 )
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					Tabs
				]
				
				+ SHorizontalBox::Slot()
				[
					SNew(SBorder)
					.Padding(FMargin(3))
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					[
						SNew(SOverlay)

						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SAssignNew(PlacementListView, SListView<TSharedPtr<FPlacementStageObject>>)
								.ListItemsSource(&FilteredItems)
								.OnGenerateRow(this, &SMapEditorToolbar::OnGenerateWidgetForItem)
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
	
	return PlaceStageObjectPanel;
}

TSharedRef<SBorder> SMapEditorToolbar::CreateEditorToolsPanel()
{
	TSharedRef<SBorder> EditorToolPanel = SNew(SBorder)
		.Padding(FMargin(10))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew ( SVerticalBox )

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateBaseMapSettings()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateWeatherSettings()
			]

			+ SVerticalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Bottom)
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(FText::FromString("Export Map Data"))
				.OnClicked( this, &SMapEditorToolbar::OnExportMapDataClicked)
			]
		];
	
	return EditorToolPanel;
}

TSharedRef<SWidget> SMapEditorToolbar::CreatePlacementGroupTab(PlacementTabInfo Info)
{
	return SNew( SCheckBox )
	.Style( FEditorStyle::Get(), "PlacementBrowser.Tab" )
	.OnCheckStateChanged( this, &SMapEditorToolbar::OnPlacementTabChanged, Info.Name )
	.IsChecked( this, &SMapEditorToolbar::GetPlacementTabCheckedState, Info.Name)
	[
		SNew( SOverlay )

		+ SOverlay::Slot()
		.VAlign( VAlign_Center )
		[
			SNew(SSpacer)
			.Size( FVector2D( 1, 30 ) )
		]

		+ SOverlay::Slot()
		.Padding( FMargin(6, 0, 15, 0) )
		.VAlign( VAlign_Center )
		[
			SNew( STextBlock )
			.TextStyle( FEditorStyle::Get(), "PlacementBrowser.Tab.Text" )
			.Text( FText::FromName(Info.Name))
		]

		+ SOverlay::Slot()
		.VAlign( VAlign_Fill )
		.HAlign( HAlign_Left )
		[
			SNew(SImage)
			.Image( this, &SMapEditorToolbar::PlacementGroupBorderImage, Info.Name)
		]
	];

}

void SMapEditorToolbar::OnPlacementTabChanged(ECheckBoxState NewState, FName CategoryName)
{
	if (NewState == ECheckBoxState::Checked)
	{
		ActivePlacementCategory = CategoryName;
		bNeedRefreshPlacementContent = true;
	}
}

const FSlateBrush* SMapEditorToolbar::PlacementGroupBorderImage(FName CategoryName) const
{
	if ( ActivePlacementCategory == CategoryName )
	{
		static FName PlacementBrowserActiveTabBarBrush( "PlacementBrowser.ActiveTabBar" );
		return FEditorStyle::GetBrush( PlacementBrowserActiveTabBarBrush );
	}
	else
	{
		return nullptr;
	}
}

ECheckBoxState SMapEditorToolbar::GetPlacementTabCheckedState(FName CategoryName) const
{
	return ActivePlacementCategory == CategoryName ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMapEditorToolbar::RefreshPlacementContent()
{
	if (!bNeedRefreshPlacementContent)
		return ;

	bNeedRefreshPlacementContent = false;

	if (PlaceableObjects.Contains(ActivePlacementCategory))
	{
		FilteredItems = PlaceableObjects[ActivePlacementCategory];
	}
	PlacementListView->RequestListRefresh();
}

void SMapEditorToolbar::RegisterPlaceableStageObjec()
{
	PlaceableObjects.Empty();
	PlaceableObjects.Add(FPlaceableCategory::GetBasicName(),			PlaceableSet());
	PlaceableObjects.Add(FPlaceableCategory::GetCharacterName(),		PlaceableSet());
	PlaceableObjects.Add(FPlaceableCategory::GetMonsterName(),			PlaceableSet());
	PlaceableObjects.Add(FPlaceableCategory::GetSpawnerName(),			PlaceableSet());
	PlaceableObjects.Add(FPlaceableCategory::GetBuildingName(),			PlaceableSet());
	PlaceableObjects.Add(FPlaceableCategory::GetPlacementObjectName(),	PlaceableSet());

	auto CheckAssetDuplicate = [&](PlaceableSet& Set, TSharedPtr<FPlacementStageObject> Obj) -> bool
	{
		return Set.ContainsByPredicate([&](TSharedPtr<FPlacementStageObject>& pStageObject) 
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
			return ;
		}

		if ((!Class->HasAllClassFlags(CLASS_NotPlaceable) &&
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
			Class->ImplementsInterface(UStageObject::StaticClass())
			// && Class->ClassGeneratedBy == nullptr // for only c++ class
			)
			|| Force)
		{
			UE_LOG(LogAtelierMapEditor, Warning, TEXT("RegisterStageObject:%s"), *Class->GetName());
			UActorFactory* Factory = GEditor->FindActorFactoryByClassForActorClass(UActorFactory::StaticClass(), Class);
			TSharedPtr<FPlacementStageObject> Obj = MakeShareable(new FPlacementStageObject(Factory, AssetData));
			PlaceableSet* TargetSet = nullptr;

			if (Class->IsChildOf(AAreaBox::StaticClass()))
			{
				TargetSet = &PlaceableObjects[FPlaceableCategory::GetSpawnerName()];
			}

			if (TargetSet != nullptr && !CheckAssetDuplicate(*TargetSet, Obj))
			{
				TargetSet->Add(Obj);
			}

			TargetSet = &PlaceableObjects[FPlaceableCategory::GetBasicName()];
			if (TargetSet != nullptr && !CheckAssetDuplicate(*TargetSet, Obj))
			{
				TargetSet->Add(Obj);
			}
		}
	};

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		const UClass* Class = *ClassIt;
		CheckAssetClass(*ClassIt, FAssetData(Class));
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.PackagePaths.Add("/Game/Blueprints/StagePlacement");
	//Filter.bRecursiveClasses = true;
	TArray< FAssetData > AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	for (FAssetData Asset : AssetList )
	{
		UE_LOG(LogAtelierMapEditor, Warning, TEXT("Detect Asset List:%s"), *Asset.PackageName.ToString());

		if (UObject* Obj = Asset.GetAsset())
		{
			CheckAssetClass(Obj->StaticClass(), Asset, true);
		}

		//if(auto GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
		//{
		//	// Convert path to just the name part
		//	const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
		//	const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

		//	UE_LOG(LogAtelierMapEditor, Warning, TEXT("Detect BP Generated Class Name:%s"), *ClassName);

		//	TArray< FAssetData > ClassAssetList;
		//	if (AssetRegistryModule.Get().GetAssetsByClass(*ClassName, ClassAssetList))
		//	{
		//		FAssetData GeneratedClassAsset = ClassAssetList[0];
		//		CheckAssetClass(GeneratedClassAsset.GetAsset()->GetClass(), Asset, true);
		//	}

		//	//TAssetSubclassOf< UObject >(FStringAssetReference(ClassObjectPath));
		//}
	}
}

TSharedRef<ITableRow> SMapEditorToolbar::OnGenerateWidgetForItem(TSharedPtr<FPlacementStageObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPlacementStageObject>>, OwnerTable)
		[
			SNew(SPlacementStageObjectAssetEntry, InItem.ToSharedRef())
			//.HighlightText(this, &SPlacementModeTools::GetHighlightText)
		];
}

TSharedRef<SWidget> SMapEditorToolbar::CreateBaseMapSettings()
{
	TSharedRef<SVerticalBox> MapSetting = SNew( SVerticalBox );
	MapSetting->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Map Setting"))
		];

	// Load Map
	MapSetting->AddSlot()
		.Padding(10, 10, 0, 0)
		.AutoHeight()
	[
		SNew(SSplitter)

		+SSplitter::Slot()
		.Value(0.2f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Select Map: "))
		]

		+SSplitter::Slot()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(MapName, STextBlock)
				.TextFlowDirection(ETextFlowDirection::LeftToRight)
				.Text(FText::FromString(MapPackagedName.IsEmpty() ? "None" : MapPackagedName))
				
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PickMapButton, SButton)
				.Text(FText::FromString("Choose..."))
				.OnClicked( this, &SMapEditorToolbar::OnSelectMapClicked)
			]
		]
	];	

	// Load Map Data
	MapSetting->AddSlot()
		.Padding(10, 10, 0, 0)
		.AutoHeight()
	[
		SNew(SSplitter)

		+SSplitter::Slot()
		.Value(0.2f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Select Map Data File: "))
		]

		+SSplitter::Slot()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(TB_MapDataFileName, STextBlock)
				.TextFlowDirection(ETextFlowDirection::LeftToRight)
				.Text(FText::FromString(MapDataFileName.IsEmpty() ? "None" : MapDataFileName))
				
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PickMapButton, SButton)
				.Text(FText::FromString("Choose..."))
				.OnClicked(this, &SMapEditorToolbar::OnSelectMapDataClicked)
			]
		]

		//+SSplitter::Slot()
		//[
		//	SNew(SHorizontalBox)

		//	//+SHorizontalBox::Slot()
		//	//.FillWidth(1.0f)
		//	//[
		//	//	SAssignNew(TB_MapDataFileName, STextBlock)
		//	//	.TextFlowDirection(ETextFlowDirection::LeftToRight)
		//	//	.Text(FText::FromString(MapDataFileName.IsEmpty() ? "None" : MapDataFileName))
		//	//	
		//	//]

		//	+SHorizontalBox::Slot()
		//	.AutoWidth()
		//	[
		//		SNew(SButton)
		//		.Text(FText::FromString("Choose Map Data..."))
		//		.OnClicked( this, &SMapEditorToolbar::OnSelectMapDataClicked)
		//		.Content()
		//		[
		//			SAssignNew(TB_MapDataFileName, STextBlock)
		//			.TextFlowDirection(ETextFlowDirection::LeftToRight)
		//			.Text(FText::FromString(MapDataFileName.IsEmpty() ? "None" : MapDataFileName))
		//		]
		//	]
		//]
	];	
	
	return MapSetting;
}

TSharedRef<SWidget> SMapEditorToolbar::CreateWeatherSettings()
{
	TSharedRef<SVerticalBox> WeatherSetting = SNew( SVerticalBox );
	WeatherSetting->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Weather Setting"))
		];

	return WeatherSetting;
}


FReply SMapEditorToolbar::OnOpenMapEditorSceneClicked()
{
	FEditorUtilities::OpenLevelByName(MAP_EDITOR_SCENE_NAME);

	//OpenEditorSceneButton->SetVisibility(EVisibility::Collapsed);
	//MainToolPanel->SetVisibility(EVisibility::Visible);

	return FReply::Handled();
}

FReply SMapEditorToolbar::OnCleanMapClicked()
{
	CleanAll();
	return FReply::Handled();
}
FReply SMapEditorToolbar::OnCleanMapConfigObjectsClicked()
{
	InternalCleanMap(false, true);
	return FReply::Handled();
}

FReply SMapEditorToolbar::OnExportMapDataClicked()
{
	InternalExportMapData();
	return FReply::Handled();
}

void SMapEditorToolbar::OnMapChanged(UWorld* World, EMapChangeType MapChangeType)
{
	bool enabled = IsToolEnable();
	if (EditorTool && EditorTool->IsValidLowLevel())
		EditorTool->DestroyEditor();
	EditorTool = nullptr;
	OpenEditorSceneButton->SetVisibility(enabled ? EVisibility::Collapsed : EVisibility::Visible);
	MainToolPanel->SetVisibility(GetToolVisibility());
	if (!HasRegisterTools)
	{
		RegisterPlaceableStageObjec();
		bNeedRefreshPlacementContent = true;
		HasRegisterTools = true;
	}
}

void SMapEditorToolbar::InternalCleanMap(bool CleanLoadedMap, bool CleanConfigObjs)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

	check(EditorWorld != nullptr);

	if (CleanLoadedMap)
	{
		TArray<class ULevelStreaming*> StreamingLevels = EditorWorld->GetStreamingLevels();
		TArray<class ULevelStreaming*> LevelToRemove;

		for (int Index = 0; Index < StreamingLevels.Num(); ++ Index)
		{
			ULevelStreaming* Level = StreamingLevels[Index];

			if (Level->GetLoadedLevel()->IsPersistentLevel())
				continue;
			LevelToRemove.Add(Level);
		}
		for (ULevelStreaming* Level : LevelToRemove)
		{
			Level->bLocked = false;
			Level->GetLoadedLevel()->bLocked = false;
			EditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
		}
	}

	if (CleanConfigObjs)
	{
		ULevel* PersistentLevel = EditorWorld->PersistentLevel;
		for (AActor* Actor : PersistentLevel->Actors)
		{
			if (Actor != nullptr && Actor->Implements<UStageObject>())
			{
				Actor->Destroy();
				EditorWorld->RemoveActor(Actor, true);
			}
		}
		PersistentLevel->Modify();
		PersistentLevel->Model->Modify();
		MapDataFileName =  "";
		FString NicePath = "None";
		TB_MapDataFileName->SetText(NicePath);
	}

	DataLevel = nullptr;
	//EditorWorld->CleanupWorld();
	EditorWorld->CleanupActors();
	GEditor->NoteSelectionChange();
}

void SMapEditorToolbar::InternalExportMapData()
{
	FMapDataModel MapData;
	int32 slotcount = 0;
	for (FActorIterator It(GEditor->GetEditorWorldContext().World()); It; ++It)
	{
		AActor* Actor = *It;
		AMonsterSpawn* editActor = Cast<AMonsterSpawn>(*It);
		if (editActor) {
			editActor->VisibilitySlot = slotcount;
			slotcount++;
		}
		if (IStageObject::IsStageObject(Actor))
		{
			UE_LOG(LogAtelierMapEditor, Warning, TEXT("StageObjectComponent:%s"), *(Actor->GetName()));
			FStageObjectSaveHandler::DumpStageObject(MapData, dynamic_cast<IStageObject *>(Actor));
		}
	}
	
	FString OutJson;
	FJsonObjectConverter::UStructToJsonObjectString(MapData, OutJson);
	UE_LOG(LogAtelierMapEditor, Warning, TEXT("Stage Map Data:%s"), *OutJson);

	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
 
	// 保存打在开对话框中选择的文件
	TArray<FString> OutFiles;
 
	const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Maps/";
	bool Ret = DesktopPlatform->SaveFileDialog(
		ParentWindowPtr,
		TEXT("Save Map Data"),
		DEFAULT_SAVE_PATH,
		FPaths::GetBaseFilename(MapDataFileName),
		TEXT("Map Data File(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FString FilePath = OutFiles[0];
		UE_LOG(LogAtelierMapEditor, Warning, TEXT("Save Stage Map Data to File:%s"), *FilePath);
		FFileHelper::SaveStringToFile(OutJson, *FilePath);
	}
}

/**
 * No use
 * Class Picker View
TSharedRef<SWidget> SMapEditorToolbar::CreateMapPicker()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	Options.bShowObjectRootClass = true;
	Options.bShowNoneOption = false;

	// This will allow unloaded blueprints to be shown.
	Options.bShowUnloadedBlueprints = true;

	return FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SMapEditorToolbar::OnSelectMap));
}
*/

FReply SMapEditorToolbar::OnSelectMapClicked()
{
	initEditorTool();
	if (bPickMapDialogOpend)
		return FReply::Handled();

	FEditorFileUtils::FOnLevelsChosen LevelsChosenDelegate = FEditorFileUtils::FOnLevelsChosen::CreateSP(this, &SMapEditorToolbar::OnSelectMap);
	FEditorFileUtils::FOnLevelPickingCancelled LevelPickingCancelledDelegate = FEditorFileUtils::FOnLevelPickingCancelled::CreateSP(this, &SMapEditorToolbar::OnSelectMapCancel);
	const bool bAllowMultipleSelection = false;
	FEditorFileUtils::OpenLevelPickingDialog(LevelsChosenDelegate, LevelPickingCancelledDelegate, bAllowMultipleSelection);

	return FReply::Handled();
}

void SMapEditorToolbar::OnSelectMap(const TArray<FAssetData>& SelectedAssets)
{
	bPickMapDialogOpend = false;

	if (SelectedAssets.Num() <= 0)
	{
		return ;
	}

	// Clean old map when load new map
	//InternalCleanMap(true, true);

	TArray<FString> PackageNames;
	for (const auto& AssetData : SelectedAssets)
	{
		PackageNames.Add(AssetData.PackageName.ToString());
	}
	//PickMapButton->SetText(FText::FromString(MapPackagedName.IsEmpty() ? "None" : MapPackagedName));

	MapPackagedName = PackageNames[0];
	MapName->SetText(MapPackagedName);

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	bool bFindLevelToConfig = false;

	if (ULevelStreaming* StreamingLevel = EditorLevelUtils::AddLevelToWorld(EditorWorld, *MapPackagedName, AddedLevelStreamingClass))
	{
		FLevelUtils::ToggleLevelLock(StreamingLevel->GetLoadedLevel());

		FString LevelPackageName = StreamingLevel->GetWorldAssetPackageName();

		if (UPackage* LevelPackage = FindPackage(nullptr, *LevelPackageName))
		{
			// Load SubLevel
			if (UWorld* OriginWorld = UWorld::FindWorldInPackage(LevelPackage))
			{
				TArray<ULevelStreaming*> Levels = OriginWorld->GetStreamingLevels();
				for (ULevelStreaming* SubLevel : Levels)
				{
					if (SubLevel && OriginWorld->IsStreamingLevelBeingConsidered(SubLevel))
					{
						FString LevelName = SubLevel->GetWorldAssetPackageName();

						if (ULevelStreaming* SubStreamingLevel = EditorLevelUtils::AddLevelToWorld(EditorWorld, *LevelName, AddedLevelStreamingClass))
						{
							if (LevelName.EndsWith("_DATA"))
							{
								DataLevel = SubStreamingLevel->GetLoadedLevel();
								bFindLevelToConfig = true;
							}
							else
							{
								FLevelUtils::ToggleLevelLock(SubStreamingLevel->GetLoadedLevel());
							}
						}
					}
				}
			}
			if (EditorWorld->SetCurrentLevel(EditorWorld->PersistentLevel))
			{
				FEditorDelegates::NewCurrentLevel.Broadcast();
			}
		}

		if (!bFindLevelToConfig)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Can't not find data level!!!"));
		}
	}
}

void SMapEditorToolbar::OnSelectMapCancel()
{
	bPickMapDialogOpend = false;
}

FReply SMapEditorToolbar::OnSelectMapDataClicked()
{
	initEditorTool();
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	TArray<FString> OutFiles;
 
	FString LAST_OPEN_PATH = EditorPlayerSettings::Get()->LastMapDataPath.FilePath;
	const FString DEFAULT_SAVE_PATH = LAST_OPEN_PATH.IsEmpty() ? FPaths::ProjectContentDir() + "Data/Maps/" : LAST_OPEN_PATH;
	bool Ret = DesktopPlatform->OpenFileDialog(
		ParentWindowPtr,
		TEXT("Load Map Data"),
		DEFAULT_SAVE_PATH,
		TEXT(""),
		TEXT("Map Data File(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FString FilePath = OutFiles[0];
		UE_LOG(LogAtelierMapEditor, Warning, TEXT("Load Stage Map Data File:%s"), *FilePath);

		MapDataFileName = FilePath;
		FString NicePath = MapDataFileName.Replace(*FPaths::ProjectContentDir(), TEXT(""));
		TB_MapDataFileName->SetText(NicePath);

		//LoadMapDataForPreview(FPaths::GetBaseFilename(*FilePath));
		LoadMapDataForPreview(FilePath);
	}
	
	return FReply::Handled();
}

void SMapEditorToolbar::LoadMapDataForPreview(const FString& MapFile)
{
	if (MapFile.IsEmpty())
		return;

	FString Path = MapFile;
	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *Path);

	FMapDataModel MapData;
	FJsonObjectConverter::JsonObjectStringToUStruct(FileContent, &MapData, 0, 0);

	//UGAGameInstance::I()->SetupMapData(MapData);
	//UGAGameInstance::I()->OnMapEditorChangeLevel.ExecuteIfBound();

	ULevel* DesireLevel = GWorld->GetCurrentLevel();
	UWorld* World = GetWorld();

	// Pop Player Spawn
	const TArray<FMapPlayerSpawnData>& PlayerSpawnPoints = MapData.PlayerSpawnPoint;
	for (FMapPlayerSpawnData PlayerSpawnPoint : PlayerSpawnPoints)
	{
		FTransform ActorTransform;
		ActorTransform.SetLocation(PlayerSpawnPoint.Position);
		ActorTransform.SetRotation(FQuat::MakeFromEuler(PlayerSpawnPoint.Rotation));
		if (APlayerSpawn* PlayerSpawn = GWorld->SpawnActor<APlayerSpawn>())
		{
			PlayerSpawn->Setup(PlayerSpawnPoint);
		}
	} // End Pop Player Spawn

	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.OverrideLevel = World->PersistentLevel;

	// Pop Monster Spawn
	const TArray<FMapMonsterSpawnData>& MonsterSpawnPoints = MapData.MonsterSpawnPoints;
	{
		for (FMapMonsterSpawnData SpawnPoint : MonsterSpawnPoints)
		{
			FString tStrPath = "/Game/Blueprints/StagePlacement/BP_MonsterSpawn.BP_MonsterSpawn_C";
			if (AMonsterSpawn* MonsterSpawn = FUtilities::SpawnBlueprintActor<AMonsterSpawn>(World, tStrPath, SpawnParameters))
				//if (AMonsterSpawn* MonsterSpawn = FUtilities::SpawnBlueprintActorByActorFactory<AMonsterSpawn>(DesireLevel->OwningWorld, Path))
			{
				MonsterSpawn->Setup(SpawnPoint);
			}
		}
	} // End Pop Monster Spawn

	// Pop Exit Area
	const TArray<FMapExitAreaData>& ExitAreas = MapData.ExitAreas;
	{
		for (const FMapExitAreaData& ExitArea : ExitAreas)
		{
			FString tStrPath = "/Game/Blueprints/StagePlacement/BP_ExitArea.BP_ExitArea_C";
			if (AExitArea* ExitAreaActor = FUtilities::SpawnBlueprintActor<AExitArea>(World, tStrPath, SpawnParameters))
			{
				ExitAreaActor->Setup(ExitArea);
				//ExitAreaActor->SetActorLocation(ExitArea.Position);
			}
		}
	}// End Pop Exit Area

	// Pop Spawn Group
	const TArray<FMapSpawnGroupData>& SpawnGroup = MapData.SpawnGroup;
	{
		for (FMapSpawnGroupData SpawnPoint : SpawnGroup)
		{
			FString tStrPath = "/Game/Blueprints/StagePlacement/BP_SpawnGroup.BP_SpawnGroup_C";
			if (ASpawnGroup* tpSpawnGroup = FUtilities::SpawnBlueprintActor<ASpawnGroup>(World, tStrPath, SpawnParameters))
				//if (AMonsterSpawn* MonsterSpawn = FUtilities::SpawnBlueprintActorByActorFactory<AMonsterSpawn>(DesireLevel->OwningWorld, Path))
			{
				tpSpawnGroup->Setup(SpawnPoint);
			}
		}
	}// End Pop Spawn Group
}
void SMapEditorToolbar::OnSearchChanged(const FText& InFilterText)
{
	
}

void SMapEditorToolbar::OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	
}

bool SMapEditorToolbar::IsToolEnable() const
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	return EditorWorld->GetMapName().Equals(MAP_EDITOR_SCENE_NAME);
}

EVisibility SMapEditorToolbar::GetToolVisibility() const
{
	return IsToolEnable() ? EVisibility::Visible: EVisibility::Collapsed;
}

void SMapEditorToolbar::CleanAll()
{
	if(EditorTool && EditorTool->IsValidLowLevel())
		EditorTool->DestroyEditor();
	InternalCleanMap(true, true);
	EditorTool = nullptr;
}

void SMapEditorToolbar::initEditorTool()
{
	if (EditorTool && EditorTool->IsValidLowLevel())
		return;

	UWorld * currentWorld = GEditor->GetAllViewportClients()[0]->GetWorld();
	for (TActorIterator<AMapEditorTool> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		EditorTool = Cast<AMapEditorTool>(*ActorItr);
		if (EditorTool)
		{
			EditorTool->SetupEditor();
			break;
		}
	}
	if (EditorTool != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Find Editor Tool"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find Editor Tool"));
	}
}