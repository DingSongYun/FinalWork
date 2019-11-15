#pragma once
#include "IDetailCustomization.h"
#include "Visibility.h"

class IDetailChildrenBuilder;
class IDetailLayoutBuilder;
class IPropertyHandle;

class FSODComponentDetailsCustomizaiton: public IDetailCustomization
{
public:
	FSODComponentDetailsCustomizaiton();
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	virtual void CustomizeDetails_Internal(class UStateOfDetailComponent* SODComponent, IDetailLayoutBuilder& DetailLayout);
	// End of IDetailCustomization interface

	void GenerateAdditionalTextureWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder, class UStateOfDetailComponent* SODComponent);
	void OnStateModelChanged(class UStateOfDetailComponent* SODComponent);

private:
	IDetailLayoutBuilder* MyDetailLayout;

};
