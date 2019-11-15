// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-30

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SComboBoxPicker.h"

struct FPreviewWeatherEntry
{
public:
    FPreviewWeatherEntry(class UGADataRow* InData)
        : WeatherData(InData)
    {}

    int32 GetWeatherId();

    FName GetWeatherName();

    FText GetEntryText();
    class UGADataRow* WeatherData;
};
typedef TSharedPtr<FPreviewWeatherEntry> FWeatherEntryPtr;
typedef SComboBoxPicker<FWeatherEntryPtr> SWeatherComboBox;
typedef TArray<FWeatherEntryPtr> FWeatherEntryArray;

class ATELIEREDITOR_API SWeatherSelectionWidget : public SCompoundWidget
{
	typedef typename TSlateDelegates<FWeatherEntryPtr>::FOnSelectionChanged FOnSelectionChanged;
public:
    SLATE_BEGIN_ARGS(SWeatherSelectionWidget)
        : _OnSelectionChanged()
    {}
        SLATE_ATTRIBUTE( int32, InitSelectedWeatherId)
        SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);
    FWeatherEntryArray& GetWeatherArray() { return WeatherEntryArray; }

    void SetCurrSelection(int32 WeatherId);

private:
    void CollectionWeatherEntry(const FString& FilterString = "");

    //~Begin: method for SComboBox
    TSharedRef<class SWidget> MakeEntryWidget(FWeatherEntryPtr InEntry) const;
    // void OnSelectionChanged(FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    void OnComboBoxSelectionChanged(FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrSelectionText() const;
    //~End: method for SComboBox

    void OnFilterTextChanged(const FText& InFilterText);

private:
    /** ComboBox widget to show character list*/
    TSharedPtr<SWeatherComboBox>	WeatherComboBox;

    /** Weather list data */
    FWeatherEntryArray WeatherEntryArray;

    /** Current selection character */
    FWeatherEntryPtr CurrSelection;

    /** OnSelection Delegate */
    FOnSelectionChanged OnSelectionChanged;
};