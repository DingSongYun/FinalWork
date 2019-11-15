#include "SStageEditMode.h"
#include "Editor.h"
#include "SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Input/SSearchBox.h"
#include "Layout/SSplitter.h"
#include "EditorStyle.h"
#include "EditorUtilities.h"
#include "FileHelpers.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "EditorLevelUtils.h"
#include "LevelUtils.h"
#include "IPluginManager.h"
#include "AssetRegistryModule.h"
#include "SPlacementStageObjectAssetEntry.h"
#include "IStageEditorInterface.h"
#include "PropertyEditorModule.h"
#include "StageEditor.h"
#include "IStageScene.h"

namespace StageEditor
{

	/** The section of the ini file we load our settings from */
	static const FString SettingsSection = TEXT("StageEditor.StageEditorSettings");

}

/*------------------------------------------------------------
SPlacementCategoryPanel
------------------------------------------------------------*/
class SPlacementsCollectionPanel : public SCompoundWidget
{
	// typedef TMap<FName, FPlacementCategory> FSourceType;
	typedef TArray<TSharedPtr<FPlacementCategory>> FSourceType;
	class PlacementTabInfo
	{
	public:
		PlacementTabInfo(FName Name, TSharedPtr<FPlacementCategory> Category)
			: Name(Name), Category(Category)
		{}
		FName Name;
		TSharedPtr<FPlacementCategory> Category;
	};
public:
	SLATE_BEGIN_ARGS(SPlacementsCollectionPanel)
	{}
	SLATE_ARGUMENT(FSourceType*, CategorySource)
	SLATE_ARGUMENT(FName, InitActiveCategory)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs)
	{
		CategorySource = InArgs._CategorySource;

		check(CategorySource && CategorySource->Num() > 0)
		ActiveCategoryPtr = (*CategorySource)[0];

		// construct widget
		TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

		TSharedRef<SVerticalBox> Tabs = SNew(SVerticalBox);
		for (auto& Element : (*this->CategorySource))
		{
			Tabs->AddSlot()
			.AutoHeight()
			[
				CreatePlacementCategoryTab(PlacementTabInfo(Element->Name, Element))
			];
		}

		this->ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSearchBox)
					.HintText(NSLOCTEXT("Stage Editor", "SearchPlaceables", "Search Stage Object"))
					.OnTextChanged(this, &SPlacementsCollectionPanel::OnSearchChanged)
					.OnTextCommitted(this, &SPlacementsCollectionPanel::OnSearchCommitted)
				]

				+ SVerticalBox::Slot()
				.Padding(0)
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
									SAssignNew(PlacementListView, SListView<FPlacementItemPtr>)
									.ListItemsSource(&FilteredItems)
									.OnGenerateRow(this, &SPlacementsCollectionPanel::GeneraterPlacementItemWidget)
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
			]
		];
	}

	// Begin SWidget
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
		if (bPendingToRefresh)
		{
			DoRefresh();
		}
	}
	// End SWidget

	void DoRefresh()
	{
		bPendingToRefresh = false;
		if (ActiveCategoryPtr.IsValid())
		{
			FilteredItems = ActiveCategoryPtr->PlacementSet;
		}
		PlacementListView->RequestListRefresh();
	}

	void RequestRefreshCategoryContent()
	{
		bPendingToRefresh = true;
	}

	TSharedRef<SWidget> CreatePlacementCategoryTab(PlacementTabInfo Info)
	{
		return SNew(SCheckBox)
		.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
		.OnCheckStateChanged(this, &SPlacementsCollectionPanel::OnPlacementTabChanged, Info.Category)
		.IsChecked(this, &SPlacementsCollectionPanel::GetPlacementTabCheckedState, Info.Name)
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
				.Image(this, &SPlacementsCollectionPanel::PlacementCategoryBorderImage, Info.Name)
			]
		];
	}

	const FSlateBrush* PlacementCategoryBorderImage(FName CategoryName) const
	{
		if (ActiveCategoryPtr.IsValid() && ActiveCategoryPtr->Name == CategoryName)
		{
			static FName PlacementBrowserActiveTabBarBrush("PlacementBrowser.ActiveTabBar");
			return FEditorStyle::GetBrush(PlacementBrowserActiveTabBarBrush);
		}

		return nullptr;
	}

	ECheckBoxState GetPlacementTabCheckedState(FName CategoryName) const
	{
		return ActiveCategoryPtr.IsValid() && ActiveCategoryPtr->Name == CategoryName
			? ECheckBoxState::Checked
			: ECheckBoxState::Unchecked;
	}

	TSharedRef<ITableRow> GeneraterPlacementItemWidget(FPlacementItemPtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		return SNew(STableRow<FPlacementItemPtr>, OwnerTable)
			[
				SNew(SPlacementStageObjectAssetEntry, InItem.ToSharedRef())
				//.HighlightText(this, &SPlacementModeTools::GetHighlightText)
			];
	}

	void OnPlacementTabChanged(ECheckBoxState NewState, TSharedPtr<FPlacementCategory> CategoryPtr)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			ActiveCategoryPtr = CategoryPtr;
			RequestRefreshCategoryContent();
		}
	}

	void OnSearchChanged(const FText& InFilterText)
	{
		// TODO: Need implements
	}

	void OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
	{
		// TODO: Need implements
	}

private:
	bool bPendingToRefresh;
	FSourceType* CategorySource;
	FPlacementSet FilteredItems;
	TSharedPtr<SListView<FPlacementItemPtr>> PlacementListView;
	TSharedPtr<FPlacementCategory> ActiveCategoryPtr;
};

/*------------------------------------------------------------
SStageEditMode
------------------------------------------------------------*/

void SStageEditMode::Construct(const FArguments& InArgs, IStageEditorInterface* InProxy)
{
	Proxy = InProxy;
	CollectPlaceableStageObject();

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = true;
		DetailsViewArgs.bHideSelectionTip = false;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.bSearchInitialKeyFocus = true;
		DetailsViewArgs.NotifyHook = IStageScene::Get()->GetStageSettingChangedHook();
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bShowModifiedPropertiesOption = false;
	}

	FStructureDetailsViewArgs StructureViewArgs;
	{
		StructureViewArgs.bShowObjects = true;
		StructureViewArgs.bShowAssets = true;
		StructureViewArgs.bShowClasses = true;
		StructureViewArgs.bShowInterfaces = false;
	}

	KeyPropertyView = EditModule.CreateStructureDetailView(DetailsViewArgs, StructureViewArgs, nullptr, FText::FromString("Stage Base Info"));
	Proxy->CustomStageDetailLayout(KeyPropertyView->GetDetailsView());

	this->ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildToolbar()
		]

		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Fill)
		.FillHeight(1.0f)
		.Padding(0, 5, 0, 0)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(0.3f)
			[
				BuildPlaceableStageObjects()
			]

			+ SSplitter::Slot()
			.Value(0.7f)
			[
				//BuildEditorMain()
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				[
					KeyPropertyView->GetWidget().ToSharedRef()
				]

				+ SVerticalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Right)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.Padding(5)
					.AutoWidth()
					[
						SNew(SButton)
						.VAlign(EVerticalAlignment::VAlign_Center)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.Text(FText::FromString("Export Config"))
						.OnClicked(this, &SStageEditMode::ExportStageConfig)
					]

					+ SHorizontalBox::Slot()
					.Padding(5)
					.AutoWidth()
					[
						SNew(SButton)
						.VAlign(EVerticalAlignment::VAlign_Center)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.Text(FText::FromString("Apply Settings"))
						.OnClicked(this, &SStageEditMode::ApplyStageData)
					]
				]
			]
		]
	];
}

FString SStageEditMode::GetName()
{
	return FString("Edit Stage");
}

FString SStageEditMode::GetDescription()
{
	return FString("Show the editing stage info");
}

TSharedRef<SWidget> SStageEditMode::BuildToolbar()
{
	TSharedRef<SWrapBox> ToolbarBox = SNew(SWrapBox)
	.UseAllottedWidth(true)
	.InnerSlotPadding(FVector2D(5, 2));

	ToolbarBox->AddSlot()
	[
		SAssignNew(ModeContainer, SBorder)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		.Padding(FMargin(4, 0, 0, 0))
	];
	/*
ToolbarBox->AddSlot()
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
				.OnClicked( this, &SStageEditMode::OnClickCleanStage )
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
						.Text( FText::FromString( "Clean Stage" ) )
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
				.OnClicked( this, &SStageEditMode::OnClickCleanStageObject )
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
	*/
	FName name;
	UpdateModeTools();
	return ToolbarBox;
}

TSharedRef<SWidget> SStageEditMode::BuildEditorMain()
{
	TSharedRef<SBorder> EditorToolPanel = SNew(SBorder)
	.Padding(FMargin(10))
	.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildBaseOpsPanel()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildExternalPanel()
		]

		+ SVerticalBox::Slot()
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		.HAlign(EHorizontalAlignment::HAlign_Right)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString("Export Stage Data"))
			.OnClicked(this, &SStageEditMode::OnClickExportStageData)
		]
	];

	return EditorToolPanel;;
}

TSharedRef<SWidget> SStageEditMode::BuildBaseOpsPanel()
{
	TSharedRef<SVerticalBox> OpsPanel = SNew(SVerticalBox);
	OpsPanel->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Stage Configuration"))
	];

	// Load Map
	OpsPanel->AddSlot()
	.Padding(10, 10, 0, 0)
	.AutoHeight()
	[
		SNew(SSplitter)

		+ SSplitter::Slot()
		.Value(0.2f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Select Map: "))
		]

		+ SSplitter::Slot()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.TextFlowDirection(ETextFlowDirection::LeftToRight)
				.Text_Lambda([this]() { return FText::FromString(StageMapName.IsEmpty() ? "None": FPaths::GetBaseFilename(StageMapName)); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Choose..."))
				.OnClicked(this, &SStageEditMode::OnClickSelectMap)
			]
		]
	];

	// Load Stage Data
	OpsPanel->AddSlot()
	.Padding(10, 10, 0, 0)
	.AutoHeight()
	[
		SNew(SSplitter)

		+ SSplitter::Slot()
		.Value(0.2f)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Select Stage Data File: "))
		]

		+ SSplitter::Slot()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.TextFlowDirection(ETextFlowDirection::LeftToRight)
				.Text_Lambda([this]() { return FText::FromString(StageDataFile.IsEmpty() ? "None" : FPaths::GetBaseFilename(StageDataFile)); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Choose..."))
			.OnClicked(this, &SStageEditMode::OnClickSelectStageDataFile)
			]
		]
	];

	return OpsPanel;
}

TSharedRef<SWidget> SStageEditMode::BuildExternalPanel()
{
	return Proxy->BuildExternalPanel();
}

TSharedRef<SWidget> SStageEditMode::BuildWeatherPanel()
{
	TSharedRef<SVerticalBox> WeatherPanel = SNew(SVerticalBox);
	// WeatherPanel->AddSlot()
	//     .AutoHeight()
	//     [
	//         // SAssignNew(PrevCharaSelWidget, SCharaSelectionWidget)
	//         //     .OnSelectionChanged_Lambda([this](FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
	//         //     {
	//         //         Proxy->SetWeather(InSelectedItem->GetWeatherId(), InSelectedItem->GetWeatherName());
	//         //         // if (InSelectedItem.IsValid())
	//         //         //     SkillEditorPtr->ChangePrevCharacter(InSelectedItem->GetCharacterId());
	//         //     })
	//     ];

	return WeatherPanel;
}

TSharedRef<SWidget> SStageEditMode::BuildPlaceableStageObjects()
{
	return SNew(SPlacementsCollectionPanel)
		.InitActiveCategory("All")
		.CategorySource(&PlacementObjects);
}

void SStageEditMode::UpdateModeTools()
{
	const TSharedPtr< const FUICommandList > CommandList = nullptr;
}

UWorld* SStageEditMode::GetWorld()
{
	FWorldContext* PIEWorldContext = GEditor->GetPIEWorldContext();
	UWorld* World = PIEWorldContext ? PIEWorldContext->World() : GEditor->GetEditorWorldContext().World();
	check(World);
	return World;
}

FReply SStageEditMode::ExportStageConfig()
{
	IStageScene::Get()->ExportStageObjConfigs();
	return FReply::Handled();
}

FReply SStageEditMode::ApplyStageData()
{
	IStageScene::Get()->ReLoadStage();
	return FReply::Handled();
}

void SStageEditMode::OnPickMapAsset(const TArray<FAssetData>& SelectedAssets)
{
	if (SelectedAssets.Num() > 0)
	{
		const FAssetData& MapAsset = SelectedAssets[0];
		SetStageMap(MapAsset.PackageName.ToString());
	}
}

void SStageEditMode::SetStageMap(const FString& MapPackageName)
{
	StageMapName = MapPackageName;
	UWorld* World = GetWorld();
	check(World);

	if (World->IsGameWorld())
	{
		FSimpleDelegate OnLoaded;
		OnLoaded.BindLambda([this]() {
			Proxy->OnStageMapChanged();
		});
		LevelLoader.AsyncLoadLevel(World, StageMapName, OnLoaded, true);
	}
	else
	{
		if (ULevelStreaming* StreamingLevel = EditorLevelUtils::AddLevelToWorld(World, *StageMapName, ULevelStreamingDynamic::StaticClass()))
		{
			FLevelUtils::ToggleLevelLock(StreamingLevel->GetLoadedLevel());

			FString LevelPackageName = StreamingLevel->GetWorldAssetPackageName();

			if (UPackage* LevelPackage = FindPackage(nullptr, *LevelPackageName))
			{
				// Load SubLevel if need
				if (UWorld* OriginWorld = UWorld::FindWorldInPackage(LevelPackage))
				{
					TArray<ULevelStreaming*> Levels = OriginWorld->GetStreamingLevels();
					for (ULevelStreaming* SubLevel : Levels)
					{
						if (SubLevel && OriginWorld->IsStreamingLevelBeingConsidered(SubLevel))
						{
							FString LevelName = SubLevel->GetWorldAssetPackageName();

							if (ULevelStreaming* SubStreamingLevel = EditorLevelUtils::AddLevelToWorld(World, *LevelName, ULevelStreamingDynamic::StaticClass()))
							{
								FLevelUtils::ToggleLevelLock(SubStreamingLevel->GetLoadedLevel());
							}
						}
					}
				}
				if (World->SetCurrentLevel(World->PersistentLevel))
				{
					FEditorDelegates::NewCurrentLevel.Broadcast();
				}
			}
		}
	}
}

void SStageEditMode::LoadStageData(const FString& InStageDataFile)
{
	if (!InStageDataFile.IsEmpty())
	{
		StageDataFile = InStageDataFile;

		// TODO: Needs implementation
	}
}

void SStageEditMode::CleanStage(bool bCleanMap, bool bCleanConfigs)
{
	UWorld* World = GetWorld();
	check(World);

	auto CleanMapFunc = [&]() {
		TArray<class ULevelStreaming*> StreamingLevels = World->GetStreamingLevels();
		TArray<class ULevelStreaming*> LevelToRemove;

		for (int Index = 0; Index < StreamingLevels.Num(); ++Index)
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

		StageMapName = "";
	};

	auto CleanConfigFunc = [&]() {
		ULevel* PersistentLevel = World->PersistentLevel;
		for (AActor* Actor : PersistentLevel->Actors)
		{
			// if (Actor != nullptr && Actor->Implements<UStageObject>())
			// {
			//     Actor->Destroy();
			//     World->RemoveActor(Actor, true);
			// }
		}
		PersistentLevel->Modify();
		PersistentLevel->Model->Modify();
		StageDataFile = "";
	};

	if (bCleanMap) CleanMapFunc();
	if (bCleanConfigs) CleanConfigFunc();

	World->CleanupActors();
	GEditor->NoteSelectionChange();
}

void SStageEditMode::CollectPlaceableStageObject()
{
	PlacementObjects.Empty();
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (Class->IsChildOf(UPlaceableCategoryFactory::StaticClass()) && !Class->HasAnyClassFlags(CLASS_Abstract))
		{
			UPlaceableCategoryFactory* Factory = Class->GetDefaultObject<UPlaceableCategoryFactory>();
			if (!Factory->GetCategoryName().IsNone())
			{
				PlacementObjects.Add(MakeShareable(new FPlacementCategory(Factory)));
			}
		}
	}

	// Add 'All' Category
	TSharedPtr<FPlacementCategory> AllCategory = MakeShareable(new FPlacementCategory("All"));
	for (auto Category : PlacementObjects)
	{
		if (Category->bPlacementsShowInAll)
		{
			for (auto Placement : Category->PlacementSet)
			{
				AllCategory->PlacementSet.AddUnique(Placement);
			}
		}
	}
	PlacementObjects.Insert(AllCategory, 0);
}

void SStageEditMode::OnEditingStageChanged(FStageScopePtr Stage)
{
	if (Stage.IsValid()) 
	{
		KeyPropertyView->SetStructureData(Stage->GetData());
	}
	else
	{
		KeyPropertyView->SetStructureData(nullptr);
	}
}

FReply SStageEditMode::OnClickSelectMap()
{
	FEditorFileUtils::FOnLevelsChosen LevelsChosenDelegate = FEditorFileUtils::FOnLevelsChosen::CreateSP(
		this, &SStageEditMode::OnPickMapAsset);
	FEditorFileUtils::FOnLevelPickingCancelled LevelPickingCancelledDelegate;
	const bool bAllowMultipleSelection = false;
	FEditorFileUtils::OpenLevelPickingDialog(LevelsChosenDelegate, LevelPickingCancelledDelegate, bAllowMultipleSelection);

	return FReply::Handled();
}

FReply SStageEditMode::OnClickSelectStageDataFile()
{
	FString LastOpenPath;
	GConfig->GetString(*StageEditor::SettingsSection, TEXT("StageDataRoot"), LastOpenPath, GEditorSettingsIni);
	const FString RootPath = LastOpenPath.IsEmpty() ? FPaths::ProjectContentDir() : LastOpenPath;
	FString InStageDataFile = FEditorUtilities::OpenPickFileDialog(RootPath,              // File Root Folder
		"Load Stage Data",      // Dialog Title
		"Stage Data File(*.txt)|*.txt" // File Type
	);
	if (!InStageDataFile.IsEmpty())
	{
		GConfig->SetString(*StageEditor::SettingsSection, TEXT("StageDataRoot"), *FPaths::GetPath(InStageDataFile), GEditorSettingsIni);
		LoadStageData(InStageDataFile);
	}
	return FReply::Handled();
}

FReply SStageEditMode::OnClickCleanStage()
{
	CleanStage(true, true);
	return FReply::Handled();
}

FReply SStageEditMode::OnClickCleanStageObject()
{
	CleanStage(false, true);
	return FReply::Handled();
}

FReply SStageEditMode::OnClickExportStageData()
{
	// TODO: Need implementation
	return FReply::Handled();
}

//void SStageEditMode::OnEditingStageChanged(TSharedPtr<FUserStructOnScope> NewStage)
//{
//	if (NewStage.IsValid())
//	{
//		EditingStageScopePtr = NewStage;
//		KeyPropertyView->SetStructureData(EditingStageScopePtr->GetData());
//	}
//	else
//	{
//		KeyPropertyView->SetStructureData(nullptr);
//	}
//}
