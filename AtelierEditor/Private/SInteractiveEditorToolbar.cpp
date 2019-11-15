#include "SInteractiveEditorToolbar.h"
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


void SInteractiveEditorToolbar::Construct(const FArguments& InArgs)
{
	FEditorUtilities::OpenLevelByName("InterractionEditor");
	ClearAllChars();
	TouchInteractiveItemDelegate.BindSP(this, &SInteractiveEditorToolbar::ClickedInteractiveItem);
	OnInteractiveDataRemove.BindSP(this, &SInteractiveEditorToolbar::RemoveInteractiveItem);

	TouchInteractionConfigItemDelegate.BindSP(this, &SInteractiveEditorToolbar::ClickedInteractionConfigItem);
	OnInteractionConfigDataRemove.BindSP(this, &SInteractiveEditorToolbar::RemoveInteractionConfigItem);
	
	ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(4, 0, 0, 4)
			.FillWidth(1)
			[
				CreateInteractiveArea()
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(4, 0, 0, 4)
			.FillWidth(1)
			[
				CreateConfigArea()
			]
		];
}

SInteractiveEditorToolbar::SInteractiveEditorToolbar()
{


}


SInteractiveEditorToolbar::~SInteractiveEditorToolbar()
{
	

}

/*创建交互物entry的区域*/
TSharedRef<SBorder> SInteractiveEditorToolbar::CreateInteractiveArea()
{
	TSharedRef<SBorder> interactiveAreaBorder = SNew(SBorder)
		.HAlign(HAlign_Fill)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[			
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()				
				.AutoHeight()
				[
					CreateInterativeEditorToolsPanel()
				]
			+ SVerticalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Fill)
				.HAlign(HAlign_Fill)
				.FillHeight(1.0f)
				.Padding(0, 5, 0, 0)
				[
					CreateInteractiveObjectPanel()
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateInteractiveDataToolsPanel()
				]	
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					LogInteractivePanel()
				]
		];

	return interactiveAreaBorder;
}
/*菜单按钮*/
TSharedRef<SWrapBox> SInteractiveEditorToolbar::CreateInterativeEditorToolsPanel()
{
	TSharedRef<SWrapBox> EditorToolPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	EditorToolPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::AddNewInteractiveItem)
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
							.Text(FEditorFontGlyphs::Plus)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Text(FText::FromString(UTF8_TO_TCHAR("创建新交互物")))
						]
					]
				]	
			]
		];

	return EditorToolPanel;
}


TSharedRef<SBorder> SInteractiveEditorToolbar::CreateInteractiveObjectPanel()
{
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	TSharedRef<SBorder> PlaceStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.HAlign(HAlign_Fill)
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
								SAssignNew(InteractiveObjectListView, SListView<TSharedPtr<FInteractiveObject>>)
								.ListItemsSource(&interactiveFilteredItems)
								.OnGenerateRow(this, &SInteractiveEditorToolbar::OnGenerateInteractiveWidgetForItem)
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

TSharedRef<SWrapBox> SInteractiveEditorToolbar::CreateInteractiveDataToolsPanel()
{
	TSharedRef<SWrapBox> DataToolsPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	DataToolsPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::ImportInteractiveData)
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
							.Text(FEditorFontGlyphs::Download)
						]
						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("导入数据...")))
						]
					]
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::ExportInteractiveData)
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
							.Text(FEditorFontGlyphs::Upload)
						]
						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("导出数据")))
						]
					]
				]	
			]
		];

	return DataToolsPanel;
}


TSharedRef<SWrapBox> SInteractiveEditorToolbar::LogInteractivePanel()
{
	TSharedRef<SWrapBox> LogToolsPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

		LogToolsPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Text(this,&SInteractiveEditorToolbar::GetInteractiveLogStr)
					.ColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 1.0f))
				]
			]
		];

	return LogToolsPanel;
}


FReply SInteractiveEditorToolbar::AddNewInteractiveItem()
{
	TSharedPtr<FInteractiveObject> Obj = MakeShareable(new FInteractiveObject());
	InteractiveObjectItems* TargetSet = nullptr;
	TargetSet = &interactiveFilteredItems;
	TargetSet->Add(Obj);
	RefreshInteractiveItemContent();
	return FReply::Handled();
}

/*保存交互物数据*/
FReply SInteractiveEditorToolbar::SaveInteractiveData()
{
	return FReply::Handled();
}

/*导入交互物数据*/
FReply SInteractiveEditorToolbar::ImportInteractiveData()
{	
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	TArray<FString> OutFiles;

	const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Interaction/";
	bool Ret = DesktopPlatform->OpenFileDialog(
		ParentWindowPtr,
		TEXT("LoadData"),
		DEFAULT_SAVE_PATH,
		TEXT(""),
		TEXT("InteractiveData(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FString FilePath = OutFiles[0];
		LoadInteractiveData(FilePath);
	}
	return FReply::Handled();
}


/*加载交互物数据*/
void SInteractiveEditorToolbar::LoadInteractiveData(const FString& MapFile)
{
	if (MapFile.IsEmpty())
		return;


	FString Path = MapFile;
	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *Path);

	FInteractiveForJsonDataNodeRoot Data;
	FJsonObjectConverter::JsonObjectStringToUStruct(FileContent, &Data, 0, 0);

	InteractiveObjectItems* TargetSet = nullptr;
	TargetSet = &interactiveFilteredItems;
	TargetSet->Empty();

	const TArray<FInteractiveForJsonData>& Items = Data.datas;
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			TSharedPtr<FInteractiveObject> it = MakeShareable(new FInteractiveObject());
			it->index = i;
			it->npcID = Items[i].npcID;
			it->interactionID = Items[i].interactionID;
			it->name = Items[i].name;
			TargetSet->Add(it);
			RefreshInteractiveItemContent();
		}
	}
}

/*导出交互物数据*/
FReply SInteractiveEditorToolbar::ExportInteractiveData()
{
	TArray<int32> ids;
	FString repeatedIds = "";
	interactiveLogStr = "";
	for (const TSharedPtr<FInteractiveObject>& Item : interactiveFilteredItems)
	{
		if (ids.Contains(Item->npcID))
		{
			repeatedIds += " " + FString::FromInt(Item->npcID);
		}
		else
		{
			ids.Add(Item->npcID);
		}
	}

	if (repeatedIds.Len() > 0)
	{
		interactiveLogStr = UTF8_TO_TCHAR(" 交互物 npcID 重复:") + repeatedIds;
		return FReply::Handled();
	}


	FInteractiveForJsonDataNodeRoot BaseData;
	for (const TSharedPtr<FInteractiveObject>& Item : interactiveFilteredItems)
	{
		this->AddInteractiveData(BaseData, Item);
	}

	FString OutJson;
	FJsonObjectConverter::UStructToJsonObjectString(BaseData, OutJson);

	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();


	TArray<FString> OutFiles;

	const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Interaction/";
	const FString MapDataFileName = "InteractiveData";
	bool Ret = DesktopPlatform->SaveFileDialog(
		ParentWindowPtr,
		TEXT("SaveData"),
		DEFAULT_SAVE_PATH,
		FPaths::GetBaseFilename(MapDataFileName),
		TEXT("InteractiveData(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FFileHelper::EEncodingOptions encodeOpt = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;
		FString FilePath = OutFiles[0];
		FFileHelper::SaveStringToFile(OutJson, *FilePath, encodeOpt);
	}

	return FReply::Handled();
}
/*添加一条交互物entry数据*/
void SInteractiveEditorToolbar::AddInteractiveData(FInteractiveForJsonDataNodeRoot& Data, TSharedPtr<FInteractiveObject> Object)
{
	if (!Object)
	{
		return;
	}
	FInteractiveForJsonData AddData;
	AddData.interactionID = Object->interactionID;
	AddData.npcID = Object->npcID;
	AddData.name = Object->name;
	Data.datas.Add(AddData);
}

/*刷新交互物entry*/
void SInteractiveEditorToolbar::RefreshInteractiveItemContent()
{
	TSharedPtr<FInteractiveObject> Obj = MakeShareable(new FInteractiveObject());
	InteractiveObjectItems* TargetSet = nullptr;
	TargetSet = &interactiveFilteredItems;
	for (int32 i = 0; i < TargetSet->Num(); i++) {
		TargetSet->GetData()[i]->index = i;
	}
	InteractiveObjectListView->RequestListRefresh();
}

void SInteractiveEditorToolbar::ClickedInteractiveItem(bool IsNew, TSharedPtr<FInteractiveObject> item)
{
	this->SelectedInteractiveIndex = item->index;
}
/*移除交互物entry*/
void  SInteractiveEditorToolbar::RemoveInteractiveItem(TSharedPtr<FInteractiveObject> item)
{
	int32 ItemIndex = item->index;
	InteractiveObjectItems * TargetSet = nullptr;
	TargetSet = &interactiveFilteredItems;
	TargetSet->RemoveAt(ItemIndex);
	RefreshInteractiveItemContent();
}

/*创建交互物entry*/
TSharedRef<ITableRow> SInteractiveEditorToolbar::OnGenerateInteractiveWidgetForItem(TSharedPtr<FInteractiveObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	InteractiveObjectAssetEntry = SNew(SInteractiveObjectAssetEntry, InItem.ToSharedRef());
	InteractiveObjectAssetEntry->SetSelectItemEventDelegate(TouchInteractiveItemDelegate);
	InteractiveObjectAssetEntry->SetRemoveDataDelegate(OnInteractiveDataRemove);

	return SNew(STableRow<TSharedPtr<FInteractiveObject>>, OwnerTable)
	[
		InteractiveObjectAssetEntry.ToSharedRef()
	];
}


FText SInteractiveEditorToolbar::GetInteractiveLogStr() const
{
	return FText::FromString(interactiveLogStr);
}

/*------------------------------------------------------------以下为编辑器交互配置区域------------------------------------------------------------------------------*/



TSharedRef<SBorder> SInteractiveEditorToolbar::CreateConfigArea()
{
	TSharedRef<SBorder> configAreaBorder = SNew(SBorder)
		.HAlign(HAlign_Fill)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateConfigEditorToolsPanel()
			]
			+ SVerticalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(HAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0, 5, 0, 0)
			[
				CreateConfigObjectPanel()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateConfigDataToolsPanel()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				LogInteractionConfigPanel()
			]
		];

	return configAreaBorder;
}


/*菜单按钮*/
TSharedRef<SWrapBox> SInteractiveEditorToolbar::CreateConfigEditorToolsPanel()
{
	TSharedRef<SWrapBox> EditorToolPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	EditorToolPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::AddNewConfigItem)
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
							.Text(FEditorFontGlyphs::Plus)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Text(FText::FromString(UTF8_TO_TCHAR("创建新配置")))
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
						.OnClicked(this, &SInteractiveEditorToolbar::AddNewSocket)
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

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("创建新挂点")))
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
						.OnClicked(this, &SInteractiveEditorToolbar::SaveConfigData)
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

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("保存数据")))
							]
						]
					]
			]
		];

	return EditorToolPanel;
}



TSharedRef<SBorder> SInteractiveEditorToolbar::CreateConfigObjectPanel()
{
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	TSharedRef<SBorder> PlaceStageObjectPanel = SNew(SBorder)
		.Padding(FMargin(3))
		.HAlign(HAlign_Fill)
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
								SAssignNew(InteractionConfigObjectListView, SListView<TSharedPtr<FInteractionConfigObject>>)
								.ListItemsSource(&interactionConfigFilteredItems)
								.OnGenerateRow(this, &SInteractiveEditorToolbar::OnGenerateInteractionObjectWidgetForItem)
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

TSharedRef<SWrapBox> SInteractiveEditorToolbar::CreateConfigDataToolsPanel()
{
	TSharedRef<SWrapBox> DataToolsPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

	DataToolsPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::ImportConfigData)
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
							.Text(FEditorFontGlyphs::Download)
						]
						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("导入数据...")))
						]
					]
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.OnClicked(this, &SInteractiveEditorToolbar::ExportConfigData)
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
							.Text(FEditorFontGlyphs::Upload)
						]
						// Import Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text(FText::FromString(UTF8_TO_TCHAR("导出数据")))
						]
					]
				]	
			]
		];

	return DataToolsPanel;
}

/*编辑器界面配置log区域*/
TSharedRef<SWrapBox> SInteractiveEditorToolbar::LogInteractionConfigPanel()
{
	TSharedRef<SWrapBox> LogToolsPanel = SNew(SWrapBox)
		.UseAllottedWidth(true)
		.InnerSlotPadding(FVector2D(5, 2));

		LogToolsPanel->AddSlot()
		.FillEmptySpace(false)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Text(this,&SInteractiveEditorToolbar::GetInteractionConfigLogStr)
					.ColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 1.0f))
				]
			]
		];

	return LogToolsPanel;
}

FReply SInteractiveEditorToolbar::AddNewConfigItem()
{
	TSharedPtr<FInteractionConfigObject> Obj = MakeShareable(new FInteractionConfigObject());
	InteractionConfigObjectItems* TargetSet = nullptr;
	TargetSet = &interactionConfigFilteredItems;
	TargetSet->Add(Obj);
	RefreshInteractionConfigItemContent();
	return FReply::Handled();
}

FReply SInteractiveEditorToolbar::AddNewSocket()
{
	if (InteractionActor == nullptr)
	{
		auto Message = NSLOCTEXT("UnrealClient", "Error", "场景中无交互物，请先选择或创建交互物");
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 5.0f;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	FSocketData data;
	InteractionActor->SpawnSocket(data);
	return FReply::Handled();
}
/*保存配置数据*/
FReply SInteractiveEditorToolbar::SaveConfigData()
{
	if (InteractionActor == nullptr) { return FReply::Handled();; }

	int32 index = SelectedInteractionConfigIndex;

	InteractionConfigObjectItems * TargetSet = nullptr;
	TargetSet = &interactionConfigFilteredItems;
	TargetSet->GetData()[index] = InteractionActor->GetData();
	InteractionActor->Refresh();

	RefreshInteractionConfigItemContent();
	return FReply::Handled();
}
/*导入配置数据*/
FReply SInteractiveEditorToolbar::ImportConfigData()
{
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	TArray<FString> OutFiles;

	const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Interaction/";
	bool Ret = DesktopPlatform->OpenFileDialog(
		ParentWindowPtr,
		TEXT("LoadData"),
		DEFAULT_SAVE_PATH,
		TEXT(""),
		TEXT("InteractionConfigData(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FString FilePath = OutFiles[0];
		LoadConfigData(FilePath);
	}
	return FReply::Handled();
}


/*读取配置数据*/
void SInteractiveEditorToolbar::LoadConfigData(const FString& MapFile)
{
	if (MapFile.IsEmpty())
		return;
	ClearAllChars();

	FString Path = MapFile;
	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *Path);

	FInteractionConfigForJsonDataNodeRoot Data;
	FJsonObjectConverter::JsonObjectStringToUStruct(FileContent, &Data, 0, 0);

	InteractionConfigObjectItems* TargetSet = nullptr;
	TargetSet = &interactionConfigFilteredItems;
	TargetSet->Empty();

	if (InteractionActor != nullptr)
	{
		InteractionActor->RemoveAllSockets();
		InteractionActor->Destroy();
		InteractionActor = nullptr;
	}

	const TArray<FInteractionConfigForJsonData>& Items = Data.datas;
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			TSharedPtr<FInteractionConfigObject> it = MakeShareable(new FInteractionConfigObject());
			it->index = i;
			it->interactionID = Items[i].interactionID;
			it->interactorDistance = Items[i].interactorDistance;
			it->name = Items[i].name;
			it->ModelName = Items[i].ModelName;
			it->socket = Items[i].socket;
			it->status = Items[i].status;
			TargetSet->Add(it);
			RefreshInteractionConfigItemContent();
		}
	}
}

/*导出交互配置数据*/
FReply SInteractiveEditorToolbar::ExportConfigData()
{
	TArray<int32> ids;
	FString repeatedIds = "";
	interactionConfigLogStr = "";
	for (const TSharedPtr<FInteractionConfigObject>& Item : interactionConfigFilteredItems)
	{
		if (ids.Contains(Item->interactionID))
		{
			repeatedIds += " " + FString::FromInt(Item->interactionID);
		}
		else
		{
			ids.Add(Item->interactionID);
		}
	}

	if (repeatedIds.Len() > 0)
	{
		interactionConfigLogStr = UTF8_TO_TCHAR(" 交互配置 interactionID 重复:") + repeatedIds;
		return FReply::Handled();
	}


	FInteractionConfigForJsonDataNodeRoot BaseData;
	for (const TSharedPtr<FInteractionConfigObject>& Item : interactionConfigFilteredItems)
	{
		this->AddConfigData(BaseData, Item);
	}

	FString OutJson;
	FJsonObjectConverter::UStructToJsonObjectString(BaseData, OutJson);

	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();


	TArray<FString> OutFiles;

	const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/Interaction/";
	const FString MapDataFileName = "InteractionConfigData";
	bool Ret = DesktopPlatform->SaveFileDialog(
		ParentWindowPtr,
		TEXT("SaveData"),
		DEFAULT_SAVE_PATH,
		FPaths::GetBaseFilename(MapDataFileName),
		TEXT("InteractionConfigData(*.txt)|*.txt"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (Ret && OutFiles.Num() > 0)
	{
		FFileHelper::EEncodingOptions encodeOpt = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;
		FString FilePath = OutFiles[0];
		FFileHelper::SaveStringToFile(OutJson, *FilePath, encodeOpt);
	}
	return FReply::Handled();
}


void SInteractiveEditorToolbar::AddConfigData(FInteractionConfigForJsonDataNodeRoot& Data, TSharedPtr<FInteractionConfigObject> Object)
{
	if (!Object)
	{
		return;
	}
	FInteractionConfigForJsonData AddData;
	AddData.interactionID = Object->interactionID;
	AddData.interactorDistance = Object->interactorDistance;
	AddData.name = Object->name;
	AddData.ModelName = Object->ModelName;
	AddData.socket = Object->socket;
	AddData.status = Object->status;

	Data.datas.Add(AddData);
}
/*刷新配置entry显示内容*/
void SInteractiveEditorToolbar::RefreshInteractionConfigItemContent()
{
	TSharedPtr<FInteractionConfigObject> Obj = MakeShareable(new FInteractionConfigObject());
	InteractionConfigObjectItems* TargetSet = nullptr;
	TargetSet = &interactionConfigFilteredItems;
	for (int32 i = 0; i < TargetSet->Num(); i++) {
		TargetSet->GetData()[i]->index = i;
	}
	InteractionConfigObjectListView->RequestListRefresh();
}
/*点击交互配置entry 创建交互物*/
void SInteractiveEditorToolbar::ClickedInteractionConfigItem(bool IsNew, TSharedPtr<const FInteractionConfigObject> item)
{
	this->SelectedInteractionConfigIndex = item->index;


	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();

	for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		AInteractionChar* EditorChar = Cast<AInteractionChar>(*ActorItr);
		if (EditorChar) {
			if (EditorChar->chatData.FilteredIndex == item->index) {
				return;
			}
			else {
				EditorChar->RemoveAllSockets();
				EditorChar->Destroy();
			}
		}
	}
	FVector objectPosition(0.f, 0.f, 0.0f);
	FRotator objectRotation(0.f, 0.f, 0.f); //in degrees
	FVector objectScale(1, 1, 1);
	FTransform objectTrasform(objectRotation, objectPosition, objectScale);
	ULevel * currentLevel = currentWorld->GetLevel(0);
	UClass * ActorClass = AInteractionChar::StaticClass();
	AActor * newActorCreated = nullptr;
	newActorCreated = GEditor->AddActor(currentLevel, ActorClass, objectTrasform, true, RF_Public | RF_Standalone | RF_Transactional);

	InteractionActor = Cast<AInteractionChar>(newActorCreated);
	InteractionActor->Setup(item);

	GEditor->EditorUpdateComponents();
	currentWorld->UpdateWorldComponents(true, false);
	GLevelEditorModeTools().MapChangeNotify();

	GEditor->SelectNone(true, true);
	GEditor->SelectActor(InteractionActor, true, true);

}
/*移除一条交互配置entry，和对应的交互物、挂点*/
void SInteractiveEditorToolbar::RemoveInteractionConfigItem(TSharedPtr<const FInteractionConfigObject> item)
{
	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
	for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		AInteractionChar* EditorChar = Cast<AInteractionChar>(*ActorItr);
		if (EditorChar) {
			if (EditorChar->chatData.interactionID == item->interactionID) {
				if (InteractionActor != nullptr && InteractionActor == EditorChar)
				{
					InteractionActor = nullptr;
				}
				EditorChar->RemoveAllSockets();
				EditorChar->Destroy();
			}
		}
	}

	int32 ItemIndex = item->index;
	InteractionConfigObjectItems * TargetSet = nullptr;
	TargetSet = &interactionConfigFilteredItems;
	TargetSet->RemoveAt(ItemIndex);
	RefreshInteractionConfigItemContent();
}

/*删除交互编辑器场景中所有的交互物和挂点*/
void SInteractiveEditorToolbar::ClearAllChars()
{
	UWorld * currentWorld = GEditor->GetLevelViewportClients()[0]->GetWorld();
	for (TActorIterator<AActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		AInteractionChar* EditorChar = Cast<AInteractionChar>(*ActorItr);
		if (EditorChar) {
			EditorChar->RemoveAllSockets();
			EditorChar->Destroy();
		}
		else
		{
			AInteractionPoint* EditorPoint = Cast<AInteractionPoint>(*ActorItr);
			if (EditorPoint) {
				EditorPoint->Destroy();
			}
		}
	}

	InteractionActor = nullptr;
}

/*创建每一条entry*/
TSharedRef<ITableRow> SInteractiveEditorToolbar::OnGenerateInteractionObjectWidgetForItem(TSharedPtr<FInteractionConfigObject> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	InteractionConfigObjectAssetEntry = SNew(SInteractionConfigObjectAssetEntry, InItem.ToSharedRef());
	InteractionConfigObjectAssetEntry->SetSelectItemEventDelegate(TouchInteractionConfigItemDelegate);
	InteractionConfigObjectAssetEntry->SetRemoveDataDelegate(OnInteractionConfigDataRemove);

	return SNew(STableRow<TSharedPtr<FInteractionConfigObject>>, OwnerTable)
	[
		InteractionConfigObjectAssetEntry.ToSharedRef()
	];
}

FText SInteractiveEditorToolbar::GetInteractionConfigLogStr() const
{
	return FText::FromString(interactionConfigLogStr);
}