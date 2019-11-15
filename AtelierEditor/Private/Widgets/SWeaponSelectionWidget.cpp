#include "Widgets/SWeaponSelectionWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Common/GADataTable.h"

#define LOCTEXT_NAMESPACE "WeaponSelectionWidget"

int32 FPreviewWeaponterEntry::GetWeaponcterId()
{
	return WeaponData->GetNumber("ID");
}

FText FPreviewWeaponterEntry::GetEntryText()
{
	return FText::FromString(WeaponData->GetAsString("Name") + "(" + WeaponData->GetAsString("ID") + ")");
}

void SWeaponSelectionWidget::Construct(const FArguments& InArgs)
{
    OnSelectionChanged = InArgs._OnSelectionChanged;

    // Collection data
    CollectionWeaponEntry();

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SAssignNew(WeaponComboBox, SWeaponComboBox)
                .OptionsSource(&WeaponEntryArray)
                .OnGenerateWidget(this, &SWeaponSelectionWidget::MakeEntryWidget)
                .OnSelectionChanged(this, &SWeaponSelectionWidget::OnComboBoxSelectionChanged)
                .OnFilterText(this, &SWeaponSelectionWidget::OnFilterTextChanged)
                [
                    SNew(STextBlock)
                        .Text(this, &SWeaponSelectionWidget::GetCurrSelectionText)
                ]
        ]
    ];
}

void SWeaponSelectionWidget::CollectionWeaponEntry(const FString& FilterString)
{
    struct NpcTableData
    {
        NpcTableData()
        {
            UGADataTable* NpcTable = UGADataTable::GetDataTable("Item_Equip");
            NpcTable->GetAllRows(Rows);
        }

        TArray<UGADataRow*> Rows;
    };
    static const NpcTableData NpcData;
    WeaponEntryArray.Empty();
    const bool bDoFilter = !FilterString.IsEmpty();
    for (UGADataRow* DataRow : NpcData.Rows)
    {
        bool bValid = true;
        TSharedPtr<FPreviewWeaponterEntry> Entry = MakeShared<FPreviewWeaponterEntry>(DataRow);
        if (bDoFilter)
        {
            bValid = Entry->GetEntryText().ToString().Contains(FilterString);
        }

        if (bValid)
        {
            WeaponEntryArray.Add(Entry);
        }
    }
}

void SWeaponSelectionWidget::SetCurrSelection(int32 WeaponId)
{
    for (auto EntryPtr : WeaponEntryArray)
    {
        if (EntryPtr->GetWeaponcterId() == WeaponId)
        {
            CurrSelection = EntryPtr;
            break;
        }
    }
}

TSharedRef<SWidget> SWeaponSelectionWidget::MakeEntryWidget(FWeaponEntryPtr InEntry) const
{
	return SNew(STextBlock)
			.MinDesiredWidth(300.f)
			.Text(InEntry->GetEntryText());
}

void SWeaponSelectionWidget::OnComboBoxSelectionChanged(FWeaponEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo)
{
    CurrSelection = InSelectedItem;
    OnSelectionChanged.ExecuteIfBound(InSelectedItem, SelectInfo);
}

FText SWeaponSelectionWidget::GetCurrSelectionText() const
{
    if (CurrSelection.IsValid())
        return CurrSelection->GetEntryText();
    return FText();
}

void SWeaponSelectionWidget::OnFilterTextChanged(const FText& InFilterText)
{
    CollectionWeaponEntry(InFilterText.ToString());
    WeaponComboBox->RefreshOptions();
}

#undef LOCTEXT_NAMESPACE