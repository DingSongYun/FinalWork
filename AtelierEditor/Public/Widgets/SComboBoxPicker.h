// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-23

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Layout/Margin.h"
#include "Styling/SlateColor.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Sound/SlateSound.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/SlateDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "Framework/Views/TableViewTypeTraits.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SSearchBox.h"

template< typename OptionType >
class SComboBoxPicker : public SComboButton
{
public:
    /** Type of list used for showing menu options. */
    typedef SListView< OptionType > SComboListType;
    /** Delegate type used to generate widgets that represent Options */
    typedef typename TSlateDelegates< OptionType >::FOnGenerateWidget FOnGenerateWidget;
    typedef typename TSlateDelegates< OptionType >::FOnSelectionChanged FOnSelectionChanged;

    SLATE_BEGIN_ARGS( SComboBoxPicker )
        : _Content()
        , _ComboBoxStyle( &FCoreStyle::Get().GetWidgetStyle< FComboBoxStyle >( "ComboBox" ) )
        , _ButtonStyle(nullptr)
        , _ItemStyle( &FCoreStyle::Get().GetWidgetStyle< FTableRowStyle >( "TableView.Row" ) )
        , _ContentPadding(FMargin(4.0, 2.0))
        , _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
        , _OptionsSource()
        , _OnFilterText()
        , _OnSelectionChanged()
        , _OnGenerateWidget()
        , _InitiallySelectedItem( nullptr)
        , _Method()
        , _MaxListHeight(450.0f)
        , _HasDownArrow( true )
        , _IsFocusable( true )
    {}
        SLATE_DEFAULT_SLOT( FArguments, Content )
        SLATE_STYLE_ARGUMENT( FComboBoxStyle, ComboBoxStyle )
        SLATE_STYLE_ARGUMENT( FButtonStyle, ButtonStyle )
        SLATE_STYLE_ARGUMENT(FTableRowStyle, ItemStyle)
        SLATE_ATTRIBUTE( FMargin, ContentPadding )
        SLATE_ATTRIBUTE( FSlateColor, ForegroundColor )
        SLATE_ARGUMENT( const TArray< OptionType >*, OptionsSource )
        SLATE_EVENT( FOnTextChanged, OnFilterText)
        SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
        SLATE_EVENT( FOnGenerateWidget, OnGenerateWidget )
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, CustomScrollbar)
        SLATE_ARGUMENT( OptionType, InitiallySelectedItem )
        SLATE_ARGUMENT( TOptional<EPopupMethod>, Method )
        SLATE_ARGUMENT(float, MaxListHeight)
        SLATE_ARGUMENT( bool, HasDownArrow )
        SLATE_ARGUMENT( bool, IsFocusable )
    SLATE_END_ARGS()

    void Construct( const FArguments& InArgs )
    {
        ItemStyle = InArgs._ItemStyle;
        const FComboButtonStyle& OurComboButtonStyle = InArgs._ComboBoxStyle->ComboButtonStyle;
        const FButtonStyle* const OurButtonStyle = InArgs._ButtonStyle ? InArgs._ButtonStyle : &OurComboButtonStyle.ButtonStyle;

        this->OnSelectionChanged = InArgs._OnSelectionChanged;
        this->OnGenerateWidget = InArgs._OnGenerateWidget;
        this->OnFilterText = InArgs._OnFilterText;

        OptionsSource = InArgs._OptionsSource;
        CustomScrollbar = InArgs._CustomScrollbar;

        // Set up picker content
        TSharedRef<SWidget> ComboBoxMenuContent =
            // SNew(SBox)
            // .MaxDesiredHeight(InArgs._MaxListHeight)
            // [
                SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(HeadSlotContainer, SVerticalBox)
				]
				
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SSearchBox)
                    .SelectAllTextWhenFocused(true)
                    .OnTextChanged(this, &SComboBoxPicker< OptionType >::OnFilterTextChanged)
                    .HintText(NSLOCTEXT("ComboBoxPicker", "Search", "Search..."))
                ]

                + SVerticalBox::Slot()
                .Padding(0.0, 5.0, 0.0, 0.0)
                .FillHeight(1.0f)
                [
                    SNew(SBox)
                    .MaxDesiredHeight(InArgs._MaxListHeight)
                    [
                        SAssignNew(this->ComboListView, SComboListType)
                        .ListItemsSource(InArgs._OptionsSource)
                        .OnGenerateRow(this, &SComboBoxPicker< OptionType >::GenerateMenuItemRow)
                        .OnSelectionChanged(this, &SComboBoxPicker< OptionType >::OnSelectionChanged_Internal)
                        .SelectionMode(ESelectionMode::Single)
                        .ExternalScrollbar(CustomScrollbar)
                    ]
                ];
            // ];

        // Set up button content
        TSharedPtr<SWidget> ButtonContent = InArgs._Content.Widget;
        if (InArgs._Content.Widget == SNullWidget::NullWidget)
        {
             SAssignNew(ButtonContent, STextBlock)
            .Text(NSLOCTEXT("SComboBoxPicker", "ContentWarning", "No Content Provided"))
            .ColorAndOpacity( FLinearColor::Red);
        }

        SComboButton::Construct( SComboButton::FArguments()
            .ComboButtonStyle(&OurComboButtonStyle)
            .ButtonStyle(OurButtonStyle)
            .Method( InArgs._Method )
            .ButtonContent()
            [
                ButtonContent.ToSharedRef()
            ]
            .MenuContent()
            [
                ComboBoxMenuContent
            ]
            .HasDownArrow( InArgs._HasDownArrow )
            .ContentPadding( InArgs._ContentPadding )
            .ForegroundColor( InArgs._ForegroundColor )
            .OnMenuOpenChanged(this, &SComboBoxPicker< OptionType >::OnMenuOpenChanged)
            .IsFocusable(InArgs._IsFocusable)
        );
        SetMenuContentWidgetToFocus(ComboListView);
        SelectedItem = InArgs._InitiallySelectedItem;
        if( TListTypeTraits<OptionType>::IsPtrValid( SelectedItem ) )
        {
            ComboListView->Private_SetItemSelection( SelectedItem, true);
            ComboListView->RequestScrollIntoView(SelectedItem, 0);
        }
    }

    void ClearSelection( )
    {
        ComboListView->ClearSelection();
    }

    void SetSelectedItem( OptionType InSelectedItem )
    {
        if (TListTypeTraits<OptionType>::IsPtrValid(InSelectedItem))
        {
            ComboListView->SetSelection(InSelectedItem);
        }
        else
        {
            ComboListView->ClearSelection();
        }
    }

    OptionType GetSelectedItem()
    {
        return SelectedItem;
    }

    void RefreshOptions()
    {
        ComboListView->RequestListRefresh();
    }

	void AddHeadSlot(TSharedPtr<SWidget> Slot) 
	{
		HeadSlotContainer->AddSlot()[Slot.ToSharedRef()];
	}

private:
    /** Generate a row for the InItem in the combo box's list (passed in as OwnerTable). Do this by calling the user-specified OnGenerateWidget */
    TSharedRef<ITableRow> GenerateMenuItemRow( OptionType InItem, const TSharedRef<STableViewBase>& OwnerTable)
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

    //** Called if the menu is closed
    void OnMenuOpenChanged(bool bOpen)
    {
        if (bOpen == false)
        {
            bControllerInputCaptured = false;

            if (TListTypeTraits<OptionType>::IsPtrValid(SelectedItem))
            {
                // Ensure the ListView selection is set back to the last committed selection
                ComboListView->SetSelection(SelectedItem, ESelectInfo::OnNavigation);
                ComboListView->RequestScrollIntoView(SelectedItem, 0);
            }

            // Set focus back to ComboBox for users focusing the ListView that just closed
            FSlateApplication::Get().ForEachUser([&](FSlateUser* User) {
                if (FSlateApplication::Get().HasUserFocusedDescendants(AsShared(), User->GetUserIndex()))
                {
                    FSlateApplication::Get().SetUserFocus(User->GetUserIndex(), AsShared(), EFocusCause::SetDirectly);
                }
            });

        }	
    }

    /** Invoked when the selection in the list changes */
    void OnSelectionChanged_Internal( OptionType ProposedSelection, ESelectInfo::Type SelectInfo )
    {
        // Ensure that the proposed selection is different
        if(SelectInfo != ESelectInfo::OnNavigation)
        {
            // Ensure that the proposed selection is different from selected
            if ( ProposedSelection != SelectedItem )
            {
                SelectedItem = ProposedSelection;
                OnSelectionChanged.ExecuteIfBound( ProposedSelection, SelectInfo );
            }
            // close combo even if user reselected item
            this->SetIsOpen( false );
        }
    }

    void OnFilterTextChanged(const FText& InFilterText)
    {
        if (OnFilterText.IsBound())
            OnFilterText.Execute(InFilterText);
    }

private:
    /** The item style to use. */
    const FTableRowStyle* ItemStyle;
    /** The item currently selected in the combo box */
    OptionType SelectedItem;
    /** The ListView that we pop up; visualized the available options. */
    TSharedPtr< SComboListType > ComboListView;
    /** The Scrollbar used in the ListView. */
    TSharedPtr< SScrollBar > CustomScrollbar;
    /** Delegate that is invoked when the selected item in the combo box changes */
    FOnSelectionChanged OnSelectionChanged;
    /** Delegate to invoke when we need to visualize an option as a widget. */
    FOnGenerateWidget OnGenerateWidget;
    /** Delegate to invoke when search box text changed. */
    FOnTextChanged OnFilterText;
    const TArray< OptionType >* OptionsSource;
    bool bControllerInputCaptured;

	TSharedPtr<SVerticalBox> HeadSlotContainer;
};