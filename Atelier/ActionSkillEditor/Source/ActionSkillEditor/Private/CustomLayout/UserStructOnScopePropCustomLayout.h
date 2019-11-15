#pragma once
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class FUserStructOnScopePropCustomLayout : public IPropertyTypeCustomization
{

public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance()
    {
        return MakeShareable(new FUserStructOnScopePropCustomLayout());
    }

    // IPropertyTypeCustomization interface
    virtual void CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils );
    virtual void CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils );
    // End of IPropertyTypeCustomization interface

private:
    struct FUserStructOnScope* GetUserStructOnScope(TSharedRef<IPropertyHandle> PropertyHandle) const;
    FText GetUserStructName(TSharedRef<IPropertyHandle> PropertyHandle) const;
    void OnPickUserStruct(UScriptStruct* ScriptStruct, TSharedRef<IPropertyHandle> PropertyHandle, IPropertyTypeCustomizationUtils* CustomizationUtils);

    // void OnInnerStructPropChanged(TSharedRef<IPropertyHandle> PropertyHandle);
    void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent, TSharedRef<IPropertyHandle> PropertyHandle);

private:
    bool bInitialized;
    TWeakObjectPtr<class UUserDefinedStruct> UserDefinedStruct;
};