// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-22

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SComboBoxPicker.h"

struct FPreviewCharacterEntry
{
public:
    FPreviewCharacterEntry(class UGADataRow* InData)
        : CharaData(InData)
    {}

    int32 GetCharacterId();

    FText GetEntryText();
    class UGADataRow* CharaData;
};
typedef TSharedPtr<FPreviewCharacterEntry> FCharaEntryPtr;
typedef SComboBoxPicker<FCharaEntryPtr> SCharaComboBox;
typedef TArray<FCharaEntryPtr> FCharaEntryArray;

class ATELIEREDITOR_API SCharaSelectionWidget : public SCompoundWidget
{
	typedef typename TSlateDelegates<FCharaEntryPtr>::FOnSelectionChanged FOnSelectionChanged;
public:
    SLATE_BEGIN_ARGS(SCharaSelectionWidget)
        : _OnSelectionChanged()
    {}
		SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);
    FCharaEntryArray& GetCharaArray() { return CharaEntryArray; }

    void SetCurrSelection(int32 CharaId);

private:
    void CollectionCharaEntry(const FString& FilterString = "");

    //~Begin: method for SComboBox
    TSharedRef<class SWidget> MakeEntryWidget(FCharaEntryPtr InEntry) const;
    // void OnSelectionChanged(FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    void OnComboBoxSelectionChanged(FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrSelectionText() const;
    //~End: method for SComboBox

    void OnFilterTextChanged(const FText& InFilterText);

private:
    /** ComboBox widget to show character list*/
    TSharedPtr<SCharaComboBox>	CharaComboBox;

    /** Character list data */
    FCharaEntryArray CharaEntryArray;

    /** Current selection character */
    FCharaEntryPtr CurrSelection;

    /** OnSelection Delegate */
    FOnSelectionChanged OnSelectionChanged;
};