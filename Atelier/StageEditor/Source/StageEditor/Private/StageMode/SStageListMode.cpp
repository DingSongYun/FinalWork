#include "StageMode/SStageListMode.h"
#include "STextBlock.h"
#include "Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "SButton.h"
#include "STextEntryPopup.h"
#include "SlateApplication.h"
#include "SBoxPanel.h"
#include "SEditableText.h"
#include "StageEditor.h"
#include "StageTable.h"
#include "PropertyCustomizationHelpers.h"

FStageItem::FStageItem(FStageScopePtr Stage)
{
	StageScope = Stage;
}

FString FStageItem::GetStageName()
{
	if (UNameProperty* Prop = Cast<UNameProperty>(StageScope->GetStructPropertyByName(FName("Name"))))
	{
		return Prop->GetPropertyValue(Prop->ContainerPtrToValuePtr<void>(StageScope->GetData()->GetStructMemory())).ToString();
	}
	return FString();
}

void SStageListMode::Construct(const FArguments& InArgs)
{
	OnChoosenItem = InArgs._OnChoosenItem;

	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar) .Thickness(FVector2D(5, 5));
	this->ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(SSearchBox)
				.HintText(NSLOCTEXT("Stage Editor", "SearchStage", "Search Stage"))
				.OnTextChanged(this, &SStageListMode::OnSearchStage)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &SStageListMode::NewStage), FText::FromString("Create a new stage"))
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(2)
		[
			SAssignNew(StageListView, SStageListView)
			.ListItemsSource(&StageListSource)
			.OnGenerateRow(this, &SStageListMode::GeneraterStageItemWidget)
			.ExternalScrollbar(ScrollBar)
			.SelectionMode(ESelectionMode::Single)
		]
	];

	CollectStageData(false);
}

void SStageListMode::CollectStageData(bool ReBuildList)
{
	StageListSource.Empty();

	TArray<FStageScopePtr> StageList = FStageEditorModule::Get().GetStateTable()->GetStageList();
	for (auto Stage : StageList) 
	{
		FStageItemPtr StageItem(new FStageItem(Stage));
		StageListSource.Add(StageItem);
	}

	if (ReBuildList) 
	{
		StageListView->RebuildList();
	}
}

void SStageListMode::OnSearchStage(const FText& InFilterText)
{
}

TSharedRef<ITableRow> SStageListMode::GeneraterStageItemWidget(FStageItemPtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FStageItemPtr>, OwnerTable)
	[
		SNew(SBorder)
		[
			SNew( SHorizontalBox )
			+ SHorizontalBox::Slot()
			.Padding(5, 5, 5, 5)
			.FillWidth(1)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew( STextBlock )
				.TextStyle( FEditorStyle::Get(), "PlacementBrowser.Asset.Name" )
				.Text( FText::FromString(InItem->GetStageName()) )
			]

			+ SHorizontalBox::Slot()
			.Padding(5, 5, 5, 5)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeUseSelectedButton(FSimpleDelegate::CreateSP(this, &SStageListMode::EditStage, InItem), FText::FromString("Edit this stage"))
			]

			+ SHorizontalBox::Slot()
			.Padding(5, 5, 5, 5)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &SStageListMode::DelStage, InItem), FText::FromString("Delete this stage"))
			]
		]
	];
}

void SStageListMode::NewStage()
{
	FStageScopePtr Stage = FStageEditorModule::Get().GetStateTable()->NewAndAddStage();
	OnChoosenItem.ExecuteIfBound(Stage);
	CollectStageData(true);
}

void SStageListMode::EditStage(FStageItemPtr StageItem)
{
	OnChoosenItem.ExecuteIfBound(StageItem->GetData());
}

void SStageListMode::DelStage(FStageItemPtr StageItem)
{
	FStageEditorModule::Get().GetStateTable()->DeleteStage(StageItem->GetData());
	CollectStageData(true);
}
