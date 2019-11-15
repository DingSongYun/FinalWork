// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#include "SFurnitureEditorToolbar.h"
#include "Editor.h"
#include "SImage.h"
#include "SButton.h"
#include "SBoxPanel.h"
#include "SSplitter.h"
#include "SListView.h"
#include "STextBlock.h"
#include "SDockTab.h"
#include "Editor/PropertyEditor/Private/SDetailSingleItemRow.h"
#include "LevelUtils.h"
#include "EditorStyle.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "EditorUtilities.h"
#include "DeclarativeSyntaxSupport.h"
#include "ActorFactories/ActorFactory.h"
#include "GameModeInfoCustomizer.h"
#include "Engine/LevelStreaming.h"
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
#include "Internationalization.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "StructBuilder.h"
#include "Components/SkeletalMeshComponent.h"
#include "EditorLuaFileHelper.h"

#define LOCTEXT_NAMESPACE "FurnitureEditor"

DEFINE_LOG_CATEGORY(LogAtelierFurnitureEditor)
const FString FURNITURE_EDITOR_SCENE_NAME = "FurnitureEditor";

TSharedRef<SVerticalBox> SGridVolumeAssetEntry::CreateAssetTransformPanel(const TSharedPtr<const FGridVolumeObject>& InItem)
{
	FTagMetaData TagMeta(TEXT("DetailRowItem"));
	TSharedRef<SVerticalBox> itemPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(STextBlock)
			.Text(InItem->mBox.IsValid() ? FText::FromName(InItem->mBox->GetFName()) : LOCTEXT("nullptr", "nullptr"))
			/*
			SNew(SDetailSingleItemRow, &Customization, true, AsShared(), OwnerTable )
			.AddMetaData<FTagMetaData>(TagMeta)
			//.ColumnSizeData(ColumnSizeData)
			*/
		]
	;
	return itemPanel;
}

void SGridVolumeAssetEntry::Construct(const FArguments & InArgs, const TSharedPtr<const FGridVolumeObject>& InItem)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ABCDetails", "ABCDetails"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0)
			[
				CreateAssetTransformPanel(InItem)
			]
		]
	];
}
/*****************************************************************/
/************************SWorkshopEditorToolbar************************/
/*****************************************************************/
SFurnitureEditorToolbar::SFurnitureEditorToolbar()
{
}

SFurnitureEditorToolbar::~SFurnitureEditorToolbar()
{
	CleanAll();
}

void SFurnitureEditorToolbar::Construct(const FArguments& InArgs)
{
	FEditorUtilities::OpenLevelByName("Editor_Furniture/Editor_Furniture");
	InitEditorTool();
	// setup the detail view settings
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea, false, NULL, false, NAME_None);
	DetailsViewArgs.bShowActorLabel = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bSearchInitialKeyFocus = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowModifiedPropertiesOption = false;
	DetailsViewArgs.bShowDifferingPropertiesOption = false;
	DetailsViewArgs.bCustomNameAreaLocation = false;
	DetailsViewArgs.bCustomFilterAreaLocation = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
	DetailsViewArgs.bShowScrollBar = true;
	DetailsViewArgs.bForceHiddenPropertyVisibility = false;
	DetailsViewArgs.bShowKeyablePropertiesOption = false;
	DetailsViewArgs.bShowAnimatedPropertiesOption = false;


	// create the detail view widget
	DetailedView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);

	// set the object to have its properties displayed
	//DetailedView->SetObject(GetWorld()->GetWorldSettings());
	DetailedView->SetObject(NULL);
	FGridVolumeObject obj = FGridVolumeObject(nullptr);
	FilterItems.Add(MakeShareable(&obj));

	ChildSlot
		[
			SNew(SBorder)
			.Cursor(EMouseCursor::Default)
			[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.Padding(2, 0, 4, 0)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0)
					.FillWidth(1.0f)
					//.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SkeletalMesh", "SkeletalMesh"))
					]
					+ SHorizontalBox::Slot()
					.Padding(4, 0, 0, 0)
					.FillWidth(1.0f)
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(USkeletalMesh::StaticClass())
						.OnObjectChanged(this, &SFurnitureEditorToolbar::OnSkeletalMeshSelected)
					]
				]
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Operation", "Operation"))				
				]
			
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					SNew(SButton)
					//.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SFurnitureEditorToolbar::OnClickReimport)
					.ContentPadding(FMargin(6, 2))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Text(FText::FromString("Reimport"))
							]
					]
				]
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					SNew(SButton)
					.OnClicked(this, &SFurnitureEditorToolbar::OnClickSaveVolumn)
					.ContentPadding(FMargin(6, 2))
					//.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ContentBrowserImportAsset")))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
								.Text(FText::FromString("SaveVolumn"))
							]
					]
				]
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Details", "Details"))
				]
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					SAssignNew(GridVolumeListView, SListView<TSharedPtr<FGridVolumeObject>>)
					.ListItemsSource(&FilterItems)
					.OnGenerateRow(this, &SFurnitureEditorToolbar::OnGenerateWidgetForVolume)
				]
			/*
			+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					DetailedView.ToSharedRef()
				]
				*/
			]
		];
}

void SFurnitureEditorToolbar::Refresh()
{
	this->Invalidate(EInvalidateWidget::All);
}

UWorld* SFurnitureEditorToolbar::GetWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

void SFurnitureEditorToolbar::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
}


void SFurnitureEditorToolbar::CleanAll()
{
	Furniture->Destroy();
	Furniture = nullptr;
	if(EditorTool)
		EditorTool->DestroyEditor();
	EditorTool = nullptr;
}

void SFurnitureEditorToolbar::InitEditorTool()
{
	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
	for (TActorIterator<AFurnitureEditorTool> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		EditorTool = Cast<AFurnitureEditorTool>(*ActorItr);
		if (EditorTool)
		{
			//EditorTool->SetupEditor();
			break;
		}
	}
	//UObject* ObjectToSpawn = FindObject<UObject>(ANY_PACKAGE, UTF8_TO_TCHAR("/Game/Blueprints/Character/BP_BaseFurniture.BP_BaseFurniture_C"));
	//UClass * Classtospawn = ObjectToSpawn->StaticClass();
	UClass* ClassToSpawn = FindObject<UClass>(ANY_PACKAGE, UTF8_TO_TCHAR("/Game/Blueprints/Character/BP_BaseFurniture.BP_BaseFurniture_C"));
	if (nullptr == ClassToSpawn)
	{
		ClassToSpawn = LoadObject<UClass>(nullptr, UTF8_TO_TCHAR("/Game/Blueprints/Character/BP_BaseFurniture.BP_BaseFurniture_C"));
	}
	const FVector Location = { 0, 0, 0 };
	const FRotator Rotation = FRotator(0, 0, 0);
	Furniture = GetWorld()->SpawnActor(ClassToSpawn, &Location, &Rotation);
}

void SFurnitureEditorToolbar::OnSkeletalMeshSelected(const FAssetData& data)
{
	if (nullptr == Furniture)
	{
		return;
	}

	USkeletalMeshComponent* comp = Furniture->FindComponentByClass<USkeletalMeshComponent>();
	USkeletalMesh* mesh = static_cast<USkeletalMesh*>(data.GetAsset());
	meshName = data.GetAsset()->GetFName().ToString();
	comp->SetSkeletalMesh(mesh);
	DetailedView->SetObject(Furniture, true);
	AStaticMeshActor* meshActor = EditorTool->GenGridVolume(Furniture, mesh);
	FGridVolumeObject obj = FGridVolumeObject(meshActor);
	FilterItems.Add(MakeShareable(&obj));
	Refresh();
}

// Reimport grid volumn from lua.
FReply SFurnitureEditorToolbar::OnClickReimport()
{
	return FReply::Handled();
}
// Save grid volumn to lua
FReply SFurnitureEditorToolbar::OnClickSaveVolumn()
{
	if (FilterItems[1].IsValid() && FilterItems[1]->mBox.IsValid())
	{
		FString meshGridInfo = GenGridInfo(FilterItems[1]->mBox);
		FEditorLuaFileHelper::SaveValueToLuaTable(meshName, meshGridInfo, *furnitureLuaTableName);
	}
	return FReply::Handled();
}

FString SFurnitureEditorToolbar::GenGridInfo(TWeakObjectPtr<AStaticMeshActor> box)
{
	FVector pos = box->GetActorLocation();
	FVector scale = box->GetActorScale3D();
	FString ret("{center = {x=" + FString::SanitizeFloat(pos.X) + ",y=" + FString::SanitizeFloat(pos.Y) + ".z=" + FString::SanitizeFloat(pos.Z) + "}" + "}"
	);
	ret.Append("{scale = {x=" + FString::SanitizeFloat(scale.X) + ",y=" + FString::SanitizeFloat(scale.Y) + ".z=" + FString::SanitizeFloat(scale.Z) + "}" + "}"	);
	return ret;
}

TSharedRef<ITableRow> SFurnitureEditorToolbar::OnGenerateWidgetForVolume(TSharedPtr<FGridVolumeObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	GridVolumeObjectAssetEntry = SNew(SGridVolumeAssetEntry, InItem.ToSharedRef());
	return SNew(STableRow<TSharedPtr<FGridVolumeObject>>, OwnerTable)
		[
			GridVolumeObjectAssetEntry.ToSharedRef()
		];
}
#undef LOCTEXT_NAMESPACE
