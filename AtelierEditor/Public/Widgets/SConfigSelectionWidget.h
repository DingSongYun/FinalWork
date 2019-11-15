#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SComboBoxPicker.h"

struct FPreviewConfigEntry
{
public:
	FPreviewConfigEntry(FString Name, FString Path);

	FString GetName() { return Name; }

	FString GetPath() { return Path; }

	FText GetEntryText() { return FText::FromString(Name + "(" + Path + ")"); };

private:
	FString Name;
	FString Path;
};
typedef TSharedPtr<FPreviewConfigEntry> FConfigEntryPtr;
typedef SComboBoxPicker<FConfigEntryPtr> SConfigComboBox;
typedef TArray<FConfigEntryPtr> FConfigEntryArray;

class ATELIEREDITOR_API SConfigSelectionWidget : public SCompoundWidget
{
	typedef typename TSlateDelegates<FConfigEntryPtr>::FOnSelectionChanged FOnSelectionChanged;
public:
    SLATE_BEGIN_ARGS(SConfigSelectionWidget) : _OnSelectionChanged() {}
	SLATE_ATTRIBUTE(FString, InitSelectedConfigPath)
	SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);

	FConfigEntryArray& GetConfigArray() { return ConfigEntryArray; }

    void SetCurrSelection(FString ConfigPath);

private:
    void CollectionWeatherEntry(const FString& FilterString = "");

    //~Begin: method for SComboBox
    TSharedRef<class SWidget> MakeEntryWidget(FConfigEntryPtr InEntry) const;

    void OnComboBoxSelectionChanged(FConfigEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo);

    FText GetCurrSelectionText() const;
    //~End: method for SComboBox

    void OnFilterTextChanged(const FText& InFilterText);

	FReply NewConfigFile();

private:
    /** ComboBox widget to show config list*/
    TSharedPtr<SConfigComboBox>	ConfigComboBox;

    /** Config list data */
    FConfigEntryArray ConfigEntryArray;

    /** Current selection character */
	FConfigEntryPtr CurrSelection;

    /** OnSelection Delegate */
    FOnSelectionChanged OnSelectionChanged;
};