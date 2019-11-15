// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-22

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SComboBoxPicker.h"

struct FPreviewWeaponterEntry
{
public:
    FPreviewWeaponterEntry(class UGADataRow* InData)
        : WeaponData(InData)
    {}

    int32 GetWeaponcterId();

    FText GetEntryText();
    class UGADataRow* WeaponData;
};
typedef TSharedPtr<FPreviewWeaponterEntry> FWeaponEntryPtr;
typedef SComboBoxPicker<FWeaponEntryPtr> SWeaponComboBox;
typedef TArray<FWeaponEntryPtr> FWeaponEntryArray;

class ATELIEREDITOR_API SWeaponSelectionWidget : public SCompoundWidget
{
	typedef typename TSlateDelegates<FWeaponEntryPtr>::FOnSelectionChanged FOnSelectionChanged;
public:
    SLATE_BEGIN_ARGS(SWeaponSelectionWidget)
        : _OnSelectionChanged()
    {}
		SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);
    FWeaponEntryArray& GetWeaponArray() { return WeaponEntryArray; }

    void SetCurrSelection(int32 WeaponId);

private:
    void CollectionWeaponEntry(const FString& FilterString = "");

    //~Begin: method for SComboBox
    TSharedRef<class SWidget> MakeEntryWidget(FWeaponEntryPtr InEntry) const;
    // void OnSelectionChanged(FWeaponEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    void OnComboBoxSelectionChanged(FWeaponEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrSelectionText() const;
    //~End: method for SComboBox

    void OnFilterTextChanged(const FText& InFilterText);

private:
    /** ComboBox widget to show Weaponcter list*/
    TSharedPtr<SWeaponComboBox>	WeaponComboBox;

    /** Weaponcter list data */
    FWeaponEntryArray WeaponEntryArray;

    /** Current selection Weaponcter */
    FWeaponEntryPtr CurrSelection;

    /** OnSelection Delegate */
    FOnSelectionChanged OnSelectionChanged;
};