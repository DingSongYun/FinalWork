#include "Widgets/SWeatherSelectionWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Common/GADataTable.h"

#define LOCTEXT_NAMESPACE "WeatherSelectionWidget"

int32 FPreviewWeatherEntry::GetWeatherId()
{
	return WeatherData->GetNumber("ID");
}

FName FPreviewWeatherEntry::GetWeatherName()
{
    return *WeatherData->GetStr("NameZh");
}

FText FPreviewWeatherEntry::GetEntryText()
{
	return FText::FromString(WeatherData->GetAsString("NameZh") + "(" + WeatherData->GetAsString("ID") + ")");
}

void SWeatherSelectionWidget::Construct(const FArguments& InArgs)
{
    OnSelectionChanged = InArgs._OnSelectionChanged;

    // Collection data
    CollectionWeatherEntry();

    ChildSlot
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        [
            SAssignNew(WeatherComboBox, SWeatherComboBox)
            .OptionsSource(&WeatherEntryArray)
        .OnGenerateWidget(this, &SWeatherSelectionWidget::MakeEntryWidget)
        .OnSelectionChanged(this, &SWeatherSelectionWidget::OnComboBoxSelectionChanged)
        .OnFilterText(this, &SWeatherSelectionWidget::OnFilterTextChanged)
        [
            SNew(STextBlock)
            .Text(this, &SWeatherSelectionWidget::GetCurrSelectionText)
        ]
        ]
        ];

    if (FWeatherEntryPtr* InitSelection = WeatherEntryArray.FindByPredicate(
        [&](FWeatherEntryPtr& Item)
        {
            return Item->GetWeatherId() == InArgs._InitSelectedWeatherId.Get();
        }))
    {
        WeatherComboBox->SetSelectedItem(*InitSelection);
    }
}

void SWeatherSelectionWidget::CollectionWeatherEntry(const FString& FilterString)
{
    struct NpcTableData
    {
        NpcTableData()
        {
            UGADataTable* NpcTable = UGADataTable::GetDataTable("Weather");
            NpcTable->GetAllRows(Rows);
        }

        TArray<UGADataRow*> Rows;
    };
    static const NpcTableData NpcData;
    WeatherEntryArray.Empty();
    const bool bDoFilter = !FilterString.IsEmpty();
    for (UGADataRow* DataRow : NpcData.Rows)
    {
        bool bValid = true;
        TSharedPtr<FPreviewWeatherEntry> Entry = MakeShared<FPreviewWeatherEntry>(DataRow);
        if (bDoFilter)
        {
            bValid = Entry->GetEntryText().ToString().Contains(FilterString);
        }

        if (bValid)
        {
            WeatherEntryArray.Add(Entry);
        }
    }
}

void SWeatherSelectionWidget::SetCurrSelection(int32 WeatherId)
{
    for (auto EntryPtr : WeatherEntryArray)
    {
        if (EntryPtr->GetWeatherId() == WeatherId)
        {
            CurrSelection = EntryPtr;
            break;
        }
    }
}

TSharedRef<SWidget> SWeatherSelectionWidget::MakeEntryWidget(FWeatherEntryPtr InEntry) const
{
	return SNew(STextBlock)
			.MinDesiredWidth(300.f)
			.Text(InEntry->GetEntryText());
}

void SWeatherSelectionWidget::OnComboBoxSelectionChanged(FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo)
{
    CurrSelection = InSelectedItem;
    OnSelectionChanged.ExecuteIfBound(InSelectedItem, SelectInfo);
}

FText SWeatherSelectionWidget::GetCurrSelectionText() const
{
    if (CurrSelection.IsValid())
        return CurrSelection->GetEntryText();
    return FText();
}

void SWeatherSelectionWidget::OnFilterTextChanged(const FText& InFilterText)
{
    CollectionWeatherEntry(InFilterText.ToString());
    WeatherComboBox->RefreshOptions();
}

#undef LOCTEXT_NAMESPACE