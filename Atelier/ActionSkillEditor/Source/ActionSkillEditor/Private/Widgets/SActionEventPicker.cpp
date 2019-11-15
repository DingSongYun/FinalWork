#include "SActionEventPicker.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditor"

FText FActionEventEntry::GetEntryText()
{
    // FNumberFormattingOptions Options;
    // Options.SetMinimumIntegralDigits(8);
    return FText::Format(LOCTEXT("ActionEventPickerText", "{0}({1})"), FText::AsNumber(EventPtr->Id), FText::FromName(EventPtr->Name));
}

void SActionEventPicker::Construct(const FArguments& InArgs)
{
    CollectionEventEntry();

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SEventPicker)
                .OptionsSource(&EventEntryArray)
                .OnGenerateWidget(this, &SActionEventPicker::MakeEntryWidget)
                .OnSelectionChanged(InArgs._OnSelectEvent)
                .OnFilterText(this, &SActionEventPicker::OnFilterTextChanged)
        ]
    ];
}

void SActionEventPicker::OnFilterTextChanged(const FText& InFilterText)
{
    CollectionEventEntry(InFilterText.ToString());
}

TSharedRef<class SWidget> SActionEventPicker::MakeEntryWidget(FEventEntryPtr InEntry) const
{
	return SNew(STextBlock)
			.MinDesiredWidth(300.f)
			.Text(InEntry->GetEntryText());
}

void SActionEventPicker::CollectionEventEntry(const FString& FilterString)
{
    const bool bDoFilter = !FilterString.IsEmpty();
    for(auto It = FActionSkillModule::Get().GetEventTable()->CreateConstIterator(); It; ++It)
    {
        const FActionEventPtr& EventPtr = It->Value;
        if (EventPtr.IsValid())
        {
            bool bValid = true;
            FEventEntryPtr Entry = MakeShared<FActionEventEntry>(EventPtr);
            if (bDoFilter)
            {
                bValid = Entry->GetEntryText().ToString().Contains(FilterString);
            }

            if (bValid)
            {
                EventEntryArray.Add(Entry);
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE