#include "Widgets/SCharaSelectionWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Common/GADataTable.h"

#define LOCTEXT_NAMESPACE "CharaSelectionWidget"

int32 FPreviewCharacterEntry::GetCharacterId()
{
	return CharaData->GetNumber("ID");
}

FText FPreviewCharacterEntry::GetEntryText()
{
	return FText::FromString(CharaData->GetAsString("NameZh") + "(" + CharaData->GetAsString("ID") + ")");
}

void SCharaSelectionWidget::Construct(const FArguments& InArgs)
{
    OnSelectionChanged = InArgs._OnSelectionChanged;

    // Collection data
    CollectionCharaEntry();

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SAssignNew(CharaComboBox, SCharaComboBox)
                .OptionsSource(&CharaEntryArray)
                .OnGenerateWidget(this, &SCharaSelectionWidget::MakeEntryWidget)
                .OnSelectionChanged(this, &SCharaSelectionWidget::OnComboBoxSelectionChanged)
                .OnFilterText(this, &SCharaSelectionWidget::OnFilterTextChanged)
                [
                    SNew(STextBlock)
                        .Text(this, &SCharaSelectionWidget::GetCurrSelectionText)
                ]
        ]
    ];
}

void SCharaSelectionWidget::CollectionCharaEntry(const FString& FilterString)
{
    struct NpcTableData
    {
        NpcTableData()
        {
            UGADataTable* NpcTable = UGADataTable::GetDataTable("NPC");
            NpcTable->GetAllRows(Rows);
        }

        TArray<UGADataRow*> Rows;
    };
    static const NpcTableData NpcData;
    CharaEntryArray.Empty();
    const bool bDoFilter = !FilterString.IsEmpty();
    for (UGADataRow* DataRow : NpcData.Rows)
    {
        bool bValid = true;
        TSharedPtr<FPreviewCharacterEntry> Entry = MakeShared<FPreviewCharacterEntry>(DataRow);
        if (bDoFilter)
        {
            bValid = Entry->GetEntryText().ToString().Contains(FilterString);
        }

        if (bValid)
        {
            CharaEntryArray.Add(Entry);
        }
    }
}

void SCharaSelectionWidget::SetCurrSelection(int32 CharaId)
{
    for (auto EntryPtr : CharaEntryArray)
    {
        if (EntryPtr->GetCharacterId() == CharaId)
        {
            CurrSelection = EntryPtr;
            break;
        }
    }
}

TSharedRef<SWidget> SCharaSelectionWidget::MakeEntryWidget(FCharaEntryPtr InEntry) const
{
	return SNew(STextBlock)
			.MinDesiredWidth(300.f)
			.Text(InEntry->GetEntryText());
}

void SCharaSelectionWidget::OnComboBoxSelectionChanged(FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo)
{
    CurrSelection = InSelectedItem;
    OnSelectionChanged.ExecuteIfBound(InSelectedItem, SelectInfo);
}

FText SCharaSelectionWidget::GetCurrSelectionText() const
{
    if (CurrSelection.IsValid())
        return CurrSelection->GetEntryText();
    return FText();
}

void SCharaSelectionWidget::OnFilterTextChanged(const FText& InFilterText)
{
    CollectionCharaEntry(InFilterText.ToString());
    CharaComboBox->RefreshOptions();
}

#undef LOCTEXT_NAMESPACE