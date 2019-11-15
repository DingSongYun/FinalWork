#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FUserStructOnScopeDetailsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FUserStructOnScopeDetailsCustomization());
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface
};