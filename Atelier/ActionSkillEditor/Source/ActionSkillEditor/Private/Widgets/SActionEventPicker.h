// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SSearchBox.h"
#include "Framework/SlateDelegates.h"
#include "Widgets/Input/SComboBox.h"
#include "Styling/CoreStyle.h"
#include "ActionSkill.h"

// template< typename OptionType >
// class SCommonPicker : public SCompoundWidget
template< typename OptionType >
class SCommonPicker : public SCompoundWidget
{
public:
    typedef SListView< OptionType > SPickerListType;
    typedef typename TSlateDelegates< OptionType >::FOnGenerateWidget FOnGenerateWidget;
    typedef typename TSlateDelegates< OptionType >::FOnSelectionChanged FOnSelectionChanged;

    SLATE_BEGIN_ARGS(SCommonPicker)
        : _OptionsSource()
        , _OnFilterText()
        , _OnSelectionChanged()
        , _OnGenerateWidget()
        , _MaxListHeight(450.0f)
        , _ItemStyle( &FCoreStyle::Get().GetWidgetStyle< FTableRowStyle >( "TableView.Row" ) )
    {}
        SLATE_ARGUMENT( const TArray< OptionType >*, OptionsSource )
        SLATE_EVENT( FOnTextChanged, OnFilterText)
        SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
        SLATE_EVENT( FOnGenerateWidget, OnGenerateWidget )
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, CustomScrollbar)
        SLATE_ARGUMENT(float, MaxListHeight)
        SLATE_STYLE_ARGUMENT(FTableRowStyle, ItemStyle)
    SLATE_END_ARGS();

    void Construct( const FArguments& InArgs )
    {
        ItemStyle = InArgs._ItemStyle;
        this->OptionsSource = InArgs._OptionsSource;
        TSharedPtr< SScrollBar > CustomScrollbar = InArgs._CustomScrollbar;
        this->OnSelectionChanged = InArgs._OnSelectionChanged;
        this->OnGenerateWidget = InArgs._OnGenerateWidget;
        this->OnFilterText = InArgs._OnFilterText;

        TSharedRef<SWidget> PickerContent =
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SSearchBox)
                .SelectAllTextWhenFocused(true)
                .OnTextChanged(this, &SCommonPicker< OptionType >::OnFilterTextChanged)
                .HintText(NSLOCTEXT("ComboBoxPicker", "Search", "Search..."))
            ]

            + SVerticalBox::Slot()
            .Padding(0.0, 5.0, 0.0, 0.0)
            .FillHeight(1.0f)
            [
                SNew(SBox)
                .MaxDesiredHeight(InArgs._MaxListHeight)
                [
                    SAssignNew(this->PickerListView, SPickerListType)
                    .ListItemsSource(InArgs._OptionsSource)
                    .OnGenerateRow(this, &SCommonPicker< OptionType >::GeneratePickerItemRow)
                    .OnSelectionChanged(this, &SCommonPicker< OptionType >::OnSelectionChanged_Internal)
                    .SelectionMode(ESelectionMode::Single)
                    .ExternalScrollbar(CustomScrollbar)
                ]
            ];

        this->ChildSlot
        [
            PickerContent
        ];
    }

    TSharedRef<ITableRow> GeneratePickerItemRow(OptionType InItem, const TSharedRef<STableViewBase>& OwnerTable)
    {
        if (OnGenerateWidget.IsBound())
        {
            return SNew(SComboRow<OptionType>, OwnerTable)
                .Style(ItemStyle)
                [
                    OnGenerateWidget.Execute(InItem)
                ];
        }
        else
        {
            return SNew(SComboRow<OptionType>, OwnerTable)
                [
                    SNew(STextBlock).Text(NSLOCTEXT("SlateCore", "ComboBoxMissingOnGenerateWidgetMethod", "Please provide a .OnGenerateWidget() handler."))
                ];

        }
    }

    void OnSelectionChanged_Internal(OptionType ProposedSelection, ESelectInfo::Type SelectInfo)
    {
        OnSelectionChanged.ExecuteIfBound( ProposedSelection, SelectInfo );
    }

    void OnFilterTextChanged(const FText& InFilterText)
    {
        if (OnFilterText.IsBound())
            OnFilterText.Execute(InFilterText);
    }
private:
    /** The item style to use. */
    const FTableRowStyle* ItemStyle;
    /** List with all options */
    TSharedPtr< SPickerListType > PickerListView;
    /** Option source */
    const TArray< OptionType >* OptionsSource;
    /** Delegate to invoke when we need to visualize an option as a widget. */
    FOnGenerateWidget OnGenerateWidget;
    /** Delegate to invoke when search box text changed. */
    FOnTextChanged OnFilterText;
    /** Delegate that is invoked when the selected item in the combo box changes */
    FOnSelectionChanged OnSelectionChanged;
};

struct FActionEventEntry
{
public:
    FActionEventEntry(FActionEventPtr InEvent)
        : EventPtr(InEvent)
    {}

    int32 GetEventId() { return EventPtr->Id; }

    FText GetEntryText();
    FActionEventPtr EventPtr;
};

typedef TSharedPtr<FActionEventEntry> FEventEntryPtr;
typedef SCommonPicker<FEventEntryPtr> SEventPicker;

class SActionEventPicker : public SCompoundWidget
{
public:
    typedef typename TSlateDelegates< FEventEntryPtr>::FOnSelectionChanged FOnSelectEvent;

    SLATE_BEGIN_ARGS(SActionEventPicker)
        : _OnSelectEvent()
    {}
        SLATE_EVENT( FOnSelectEvent, OnSelectEvent )
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);
    void OnFilterTextChanged(const FText& InFilterText);
    TSharedRef<class SWidget> MakeEntryWidget(FEventEntryPtr InEntry) const;
    void CollectionEventEntry(const FString& FilterString = "");
private:
    TArray<FEventEntryPtr> EventEntryArray;
};