#include "UserStructOnScopePropCustomLayout.h"
#include "UserStructOnScope.h"
#include "PropertyHandle.h"
#include "IDetailChildrenBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/SActionEventPicker.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "IPropertyUtilities.h"
#include "Fonts/SlateFontInfo.h"
#include "EditorStyleSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/StructureEditorUtils.h"

DECLARE_DELEGATE_OneParam(FOnPickStruct, UScriptStruct* /* ScriptStruct */)

class SUserStructPicker : public SCompoundWidget
{
private:
    FOnPickStruct OnPickStruct;
    TAttribute<UClass*> BaseStructCls;
public:
    SLATE_BEGIN_ARGS( SUserStructPicker )
        : _UserStructName()
        , _BaseStruct(UScriptStruct::StaticClass())
        , _Font( FEditorStyle::GetFontStyle( TEXT("NormalFont") ) )
    {}
        SLATE_ATTRIBUTE( FText, UserStructName )
        SLATE_ATTRIBUTE( UClass*, BaseStruct )
        SLATE_ATTRIBUTE( FSlateFontInfo, Font )
        SLATE_EVENT(FOnPickStruct, OnPickStruct)
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs)
    {
        this->OnPickStruct = InArgs._OnPickStruct;
        this->BaseStructCls = InArgs._BaseStruct;

        this->ChildSlot
        [
            SNew( SComboButton )
            .OnGetMenuContent(this, &SUserStructPicker::GetMenuContent)
            .ContentPadding(0)
            .ButtonContent()
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                [
                    SNew(STextBlock)
                    .Text( InArgs._UserStructName )
                    .Font( InArgs._Font )
                ]
            ]
        ];
    }

    TSharedRef<SWidget> GetMenuContent()
    {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
        FAssetPickerConfig AssetPickerConfig;
        {
            AssetPickerConfig.Filter.ClassNames.Add( BaseStructCls.Get() ? BaseStructCls.Get()->GetFName() : UScriptStruct::StaticClass()->GetFName());
            AssetPickerConfig.Filter.bRecursiveClasses = true;
            AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
            AssetPickerConfig.bAllowNullSelection = false;
            AssetPickerConfig.bFocusSearchBoxWhenOpened = true;
            AssetPickerConfig.bAllowDragging = false;
            AssetPickerConfig.SaveSettingsName = TEXT("AssetPropertyPicker");
            AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda(
                [&](const FAssetData& AssetData) {
                if (UObject* NotifyStruct = AssetData.GetAsset())
                {
                    if (UScriptStruct* SStruct = Cast<UScriptStruct>(NotifyStruct))
                    {
                        this->OnPickStruct.ExecuteIfBound(SStruct);
                    }
                }
            });
        }
        return SNew(SBox)
        .MinDesiredWidth(300.0f)
        .MaxDesiredHeight(400.0f)
        [
            ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
        ];
    }
};

FUserStructOnScope* FUserStructOnScopePropCustomLayout::GetUserStructOnScope(TSharedRef<IPropertyHandle> PropertyHandle) const
{
    void* RawData = nullptr;
    PropertyHandle->GetValueData(RawData);
    if (RawData)
    {
        return static_cast<FUserStructOnScope*>(RawData);
    }

    return nullptr;
}

void FUserStructOnScopePropCustomLayout::CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
    bool bShowStructPicker = true;
    FUserStructOnScope* UserStructScope = GetUserStructOnScope(PropertyHandle);
    bShowStructPicker = UserStructScope->CanChangeUserStructType();

    if (bShowStructPicker)
    {
        HeaderRow
        .NameContent()
        [
            PropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        [
            SNew(SUserStructPicker)
            .UserStructName(this, &FUserStructOnScopePropCustomLayout::GetUserStructName, PropertyHandle)
            .BaseStruct(UserStructScope->GetUserStructBaseType())
            .OnPickStruct(this, &FUserStructOnScopePropCustomLayout::OnPickUserStruct, PropertyHandle, &CustomizationUtils)
        ];
    }
    else
    {
        HeaderRow
        .NameContent()
        [
            PropertyHandle->CreatePropertyNameWidget()
        ];
    }
}

void FUserStructOnScopePropCustomLayout::CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
    if (bInitialized == false)
    {
        IDetailLayoutBuilder& DetailLayout = ChildBuilder.GetParentCategory().GetParentLayout();
        const TArray<TWeakObjectPtr<UObject>>& Objects = DetailLayout.GetSelectedObjects();

        if (Objects.Num() >= 1)
        {
            UserDefinedStruct = CastChecked<UUserDefinedStruct>(Objects[0].Get());
        }
        if (UserDefinedStruct.IsValid())
        {
            DetailLayout.GetDetailsView()->OnFinishedChangingProperties().AddSP(
                this, &FUserStructOnScopePropCustomLayout::OnFinishedChangingProperties, PropertyHandle);
        }
        bInitialized = true;
    }

    if (FUserStructOnScope* UserStructScope = GetUserStructOnScope(PropertyHandle))
    {
        if (UserStructScope->IsValid())
        {
            for (TSharedPtr<IPropertyHandle> ChildHandle : PropertyHandle->AddChildStructure(UserStructScope->GetData().ToSharedRef()))
            {
                // FSimpleDelegate OnPropChangedDelegate;
                // OnPropChangedDelegate.BindSP(this, &FUserStructOnScopePropCustomLayout::OnInnerStructPropChanged, PropertyHandle);
                // ChildHandle->SetOnPropertyValueChanged(OnPropChangedDelegate);
                // ChildHandle->SetOnChildPropertyValueChanged(OnPropChangedDelegate);
                ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
            }
        }
    }
}

FText FUserStructOnScopePropCustomLayout::GetUserStructName(TSharedRef<IPropertyHandle> PropertyHandle) const
{
    if (FUserStructOnScope* UserStructScope = GetUserStructOnScope(PropertyHandle))
    {
        if (UserStructScope->IsValid())
        {
            return FText::FromName(UserStructScope->GetStruct()->GetFName());
        }
    }
    return FText::FromString("None");
}

void FUserStructOnScopePropCustomLayout::OnPickUserStruct(UScriptStruct* ScriptStruct, TSharedRef<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils)
{
    if (FUserStructOnScope* UserStructScope = GetUserStructOnScope(PropertyHandle))
    {
        UserStructScope->Reset();
        UserStructScope->Initialize(*ScriptStruct);
        CustomizationUtils->GetPropertyUtilities()->ForceRefresh();
        PropertyHandle->NotifyFinishedChangingProperties();
    }
}

// void FUserStructOnScopePropCustomLayout::OnInnerStructPropChanged(TSharedRef<IPropertyHandle> PropertyHandle)
// {
//     PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
//     PropertyHandle->NotifyFinishedChangingProperties();
// }

void FUserStructOnScopePropCustomLayout::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent, TSharedRef<IPropertyHandle> PropertyHandle)
{
    FUserStructOnScope* UserStructScope = GetUserStructOnScope(PropertyHandle);
    if (UserStructScope && UserStructScope->IsValid())
    {
        UStruct* OwnerStruct = PropertyChangedEvent.MemberProperty->GetOwnerStruct();

        check(PropertyChangedEvent.MemberProperty && OwnerStruct);

        if ( ensure(OwnerStruct == UserStructScope->GetStruct()) )
        {
            const UProperty* DirectProperty = PropertyChangedEvent.MemberProperty;
            while (DirectProperty && !Cast<const UUserDefinedStruct>(DirectProperty->GetOuter()))
            {
                DirectProperty = Cast<const UProperty>(DirectProperty->GetOuter());
            }
            ensure(nullptr != DirectProperty);

            if (DirectProperty)
            {
                FString DefaultValueString;
                bool bDefaultValueSet = false;
                {
                    TSharedPtr<FStructOnScope> StructData = UserStructScope->GetData();
                    if (StructData.IsValid() && StructData->IsValid())
                    {
                        bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(DirectProperty, StructData->GetStructMemory(), DefaultValueString);
                    }
                }

                const FGuid VarGuid = FStructureEditorUtils::GetGuidForProperty(DirectProperty);
                if (bDefaultValueSet && VarGuid.IsValid())
                {
                    FStructureEditorUtils::ChangeVariableDefaultValue(UserDefinedStruct.Get(), VarGuid, DefaultValueString);
                }
            }
        }
    }
}