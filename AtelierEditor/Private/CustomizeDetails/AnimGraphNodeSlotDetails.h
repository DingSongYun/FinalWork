// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-03-07

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class IPropertyHandle;
class USkeleton;

/**
 * Customizes a DataTable asset to use a dropdown
 */
class FAnimGraphNodeSlotDetails : public IDetailCustomization
{
public:
	FAnimGraphNodeSlotDetails();

	static TSharedRef<IDetailCustomization> MakeInstance() 
	{
		return MakeShareable( new FAnimGraphNodeSlotDetails() );
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
	
private:
	// property of the two 
	TSharedPtr<IPropertyHandle> SlotNodeNamePropertyHandle;

	// slot node names
	TSharedPtr<class STextComboBox>	SlotNameComboBox;
	TArray< TSharedPtr< FString > > SlotNameComboListItems;
	TArray< FName > SlotNameList;
	FName SlotNameComboSelectedName;

	void OnSlotNameChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnSlotListOpening();

	// slot node names buttons
	FReply OnOpenAnimSlotManager();

	void RefreshComboLists(bool bOnlyRefreshIfDifferent = false);

	USkeleton* Skeleton;
};

/**Empty Derived Class*/
class FAnimGraphNodeSlotBlendDetails : public FAnimGraphNodeSlotDetails
{
};


