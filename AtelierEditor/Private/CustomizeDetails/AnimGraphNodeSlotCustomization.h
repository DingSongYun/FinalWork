#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class IPropertyHandle;
class USkeleton;

/**
 * Customizes a DataTable asset to use a dropdown
 */
class FAnimGraphNodeSlotBlendDetails : public IDetailCustomization
{
public:
	FAnimGraphNodeSlotBlendDetails();

	static TSharedRef<IDetailCustomization> MakeInstance() 
	{
		return MakeShareable( new FAnimGraphNodeSlotBlendDetails() );
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