#include "SActionEventStructTab.h"
#include "Widgets/Views/SListView.h"
#include "AssetRegistryModule.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "ActionSkill.h"
#include "ActionSkillEditor.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Views/STableRow.h"
#include "StructureDetailCustomization.h"
#include "ActionSkillDefineMode.h"

void SActionEventStructTab::Construct(const FArguments& InArgs, FActionSkillDefineMode* DefineMode)
{
    SkillEdDefineModePtr = DefineMode;
    this->ChildSlot
    [
        SNew(SBox)
        .MaxDesiredHeight(450.f)
        [
            SAssignNew(this->EventStructureListView, SEventStructListType)
            .ListItemsSource(&OptionSource)
            .OnGenerateRow(this, &SActionEventStructTab::GenerateMenuItemRow)
            .SelectionMode(ESelectionMode::None)
        ]
    ];
    CollectionActionEventStructure();

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SActionEventStructTab::OnNewAsset);
    AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SActionEventStructTab::OnDeleteAsset);
    AssetRegistryModule.Get().OnAssetRenamed().AddSP(this, &SActionEventStructTab::OnRenameAsset);
}

void SActionEventStructTab::CollectionActionEventStructure()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

    FARFilter Filter;
    // Filter.ClassNames.Add(UScriptStruct::StaticClass()->GetFName());
    Filter.ClassNames.Add(UActionEventStructType::StaticClass()->GetFName());
    Filter.bRecursivePaths = true;
    Filter.bRecursiveClasses = true;
    // Filter.TagsAndValues(FName("ActionSkillNotify"), "true");

    // ActionSkillEditor.Pin()->GetCurrentModePtr();
    OptionSource.Empty();
    TArray< FAssetData > AssetList;
    AssetRegistryModule.Get().GetAssets(Filter, AssetList);
    for (auto AssetData : AssetList)
    {
        OptionSource.Add(AssetData.GetAsset());
        SkillEdDefineModePtr->AddObjectToSave(AssetData.GetAsset());
    }
}

TSharedRef<ITableRow> SActionEventStructTab::GenerateMenuItemRow( StructureTypePtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    {
        DetailsViewArgs.bUpdatesFromSelection = false;
        DetailsViewArgs.bLockable = false;
        DetailsViewArgs.bAllowSearch = false;
        DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
        DetailsViewArgs.bHideSelectionTip = true;
        DetailsViewArgs.bShowOptions = false;
    }

    TSharedPtr<class IDetailsView> PropertyView = EditModule.CreateDetailView(DetailsViewArgs);
    {
        FOnGetDetailCustomizationInstance LayoutStructDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FUserDefinedStructureDetails::MakeInstance);
        PropertyView->RegisterInstancedCustomPropertyLayout(UUserDefinedStruct::StaticClass(), LayoutStructDetails);
        PropertyView->SetObject(InItem);
    }

    return SNew(STableRow<StructureTypePtr>, OwnerTable)
        .Style(&FCoreStyle::Get().GetWidgetStyle< FTableRowStyle >( "TableView.Row" ))
        [
            PropertyView.ToSharedRef()
        ];
}

void SActionEventStructTab::InternalOnAssetModified(const FAssetData& InAssetData)
{
    if (InAssetData.GetClass() == UActionEventStructType::StaticClass())
    {
        RebuildEventStructureList();
    }
}

void SActionEventStructTab::RebuildEventStructureList()
{
    CollectionActionEventStructure();
    EventStructureListView->RebuildList();
}