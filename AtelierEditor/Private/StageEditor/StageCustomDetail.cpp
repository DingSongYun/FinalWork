#include "StageCustomDetail.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/SWeatherSelectionWidget.h"
#include "Widgets/SConfigSelectionWidget.h"
#include "Types/WeatherStruct.h"
#include "StageConfigStruct.h"

template<typename T>
T* GetProperValueAsStruct(TSharedRef<IPropertyHandle> PropertyHandle)
{
    void* RawData = nullptr;
    PropertyHandle->GetValueData(RawData);
    if (RawData)
    {
        return static_cast<T*>(RawData);
    }

    return nullptr;
}

void FWeatherDetailsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

    FStageWeather* Weather = GetProperValueAsStruct<FStageWeather>(PropertyHandle);

    HeaderRow
    .NameContent()
    [
        PropertyHandle->CreatePropertyNameWidget()
    ]
    .ValueContent()
    [
        SNew(SWeatherSelectionWidget)
            .InitSelectedWeatherId(Weather->Id)
            .OnSelectionChanged_Lambda([Weather, PropertyHandle](FWeatherEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
            {
                PropertyHandle->NotifyPreChange();
                // Set Weather
                if (Weather)
                {
                    Weather->Id = InSelectedItem->GetWeatherId();
                }
                PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
            })
    ];
}

void FWeatherDetailsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    // No show any childre widget here
}

void FConfigDetailsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow & HeaderRow, IPropertyTypeCustomizationUtils & CustomizationUtils)
{
	FStageConfig* Config = GetProperValueAsStruct<FStageConfig>(PropertyHandle);

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SConfigSelectionWidget)
		.InitSelectedConfigPath(Config->FilePath)
		.OnSelectionChanged_Lambda([Config, PropertyHandle](FConfigEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo)
		{
			PropertyHandle->NotifyPreChange();
			// Set Weather
			if (Config)
			{
				Config->FilePath = InSelectedItem->GetPath();
			}
			PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		})
	];
}

void FConfigDetailsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder & ChildBuilder, IPropertyTypeCustomizationUtils & CustomizationUtils)
{
	// No show any childre widget here
}
