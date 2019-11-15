// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-08-12
#include "SActionKeyDetailTab.h"
#include "ActionSkillEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "IStructureDetailsView.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "ActionSkillEditor.h"
#include "ActionSkill.h"

#define LOCTEXT_NAMESPACE "SAnimationBlendSpaceGridWidget"

class FActionKeyNotifyCustomization : public IPropertyTypeCustomization
{
public:
	void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{

		const bool bDisplayResetToDefault = false;
		const FText DisplayNameOverride = FText::GetEmpty();
		// const FText DisplayNameOverride = FText::FromString("Test Name");
		const FText DisplayToolTipOverride = FText::GetEmpty();
		HeaderRow
			.NameContent()
			[
				PropertyHandle->CreatePropertyNameWidget( DisplayNameOverride, DisplayToolTipOverride, bDisplayResetToDefault )
			];
	}

	void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
	{
		void* RawData = nullptr;
		StructPropertyHandle->GetValueData(RawData);
		if (RawData)
		{
			FUserStructOnScope* NotifyStruct = static_cast<FUserStructOnScope*>(RawData);

			if (NotifyStruct->IsValid())
			{
				// IDetailCategoryBuilder& DefalutValueCategory = ChildBuilder.GetParentCategory();
				// for (TFieldIterator<UProperty> It(NotifyStruct->GetStruct()); It; ++It)
				// {
				// 	DefalutValueCategory.AddExternalStructureProperty(NotifyStruct->GetData().ToSharedRef(), (*It)->GetFName());
				// }
				// DefalutValueCategory.AddAllExternalStructureProperties(NotifyStruct->GetData().ToSharedRef());
				for (TSharedPtr<IPropertyHandle> ChildHandle : StructPropertyHandle->AddChildStructure(NotifyStruct->GetData().ToSharedRef()))
				{
					ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
				}
			}
		}
	}
};

class FActionKeyFrameDetailsCustomizaiton : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FActionKeyFrameDetailsCustomizaiton());
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
	{
		TArray<TSharedPtr<FStructOnScope>> StructsBeingCustomized;
		DetailLayout.GetStructsBeingCustomized(StructsBeingCustomized);
		if (StructsBeingCustomized.Num() > 0)
		{
			if (FActionKeyFrame* KeyFrame = (FActionKeyFrame*)(StructsBeingCustomized[0]->GetStructMemory()))
			{
				CustomizeDetails_Internal(KeyFrame, DetailLayout);
			}
		}
	}

	virtual void CustomizeDetails_Internal(FActionKeyFrame* KeyFrame, IDetailLayoutBuilder& DetailLayout)
	{
		IDetailCategoryBuilder& KFCategory = DetailLayout.EditCategory("KeyFrame", FText::GetEmpty(), ECategoryPriority::Important);
		KFCategory.AddProperty(GET_MEMBER_NAME_CHECKED(FActionKeyFrame, Name));
		KFCategory.AddProperty(GET_MEMBER_NAME_CHECKED(FActionKeyFrame, Time));

		IDetailCategoryBuilder& EventCategory = DetailLayout.EditCategory("KeyEvent", FText::GetEmpty(), ECategoryPriority::Important);
		FActionEventPtr ActionEvent = FActionSkillModule::Get().GetActionEventById(KeyFrame->EventId);
		if (ActionEvent.IsValid())
		{
			auto ActionEventScope = MakeShareable(new FStructOnScope(FActionEvent::StaticStruct(), (uint8*)ActionEvent.Get()));
			EventCategory.AddExternalStructureProperty(ActionEventScope, GET_MEMBER_NAME_CHECKED(FActionEvent, Name));

			if (ActionEvent->Params.IsValid())
			{
				EventCategory.AddAllExternalStructureProperties(ActionEvent->Params.GetData().ToSharedRef());
			}
			// Disable SubEvent
			// EventCategory.AddExternalStructureProperty(ActionEventScope, GET_MEMBER_NAME_CHECKED(FActionEvent, SubEvents), EPropertyLocation::Advanced);
		}

		IDetailCategoryBuilder& EventArgsCategory = DetailLayout.EditCategory("KeyEventArgs", FText::GetEmpty(), ECategoryPriority::Important);
		EventArgsCategory.AddProperty(GET_MEMBER_NAME_CHECKED(FActionKeyFrame, EventArgs));
	}
	// End of IDetailCustomization interface
};

void SActionKeyDetailTab::Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr)
{
	check(InEditorPtr.IsValid());

	ActionSkillEditor = InEditorPtr;
	ActionSkillEditor.Pin()->OnEditingKeyFrameSet().AddSP(this, &SActionKeyDetailTab::OnEditingKeyFrameChanged);
	ActionSkillEditor.Pin()->OnEditingEventChanged().AddSP(this, &SActionKeyDetailTab::OnEditingEventChanged);
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = true;
		DetailsViewArgs.bHideSelectionTip = false;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.bSearchInitialKeyFocus = true;
		DetailsViewArgs.NotifyHook = this;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bShowModifiedPropertiesOption = false;
	}

	FStructureDetailsViewArgs StructureViewArgs;
	{
		StructureViewArgs.bShowObjects = true;
		StructureViewArgs.bShowAssets = true;
		StructureViewArgs.bShowClasses = true;
		StructureViewArgs.bShowInterfaces = false;
	}

	KeyPropertyView = EditModule.CreateStructureDetailView(DetailsViewArgs, StructureViewArgs, nullptr, LOCTEXT("Action", "KeyFrame"));

	// FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// PropertyModule.RegisterCustomClassLayout(UStateOfDetailComponent::StaticClass()->GetFName(),
	// 	FOnGetDetailCustomizationInstance::CreateStatic(&FSODComponentDetailsCustomizaiton::MakeInstance));
	
	EditModule.RegisterCustomClassLayout(FActionKeyFrame::StaticStruct()->GetFName(), 
		FOnGetDetailCustomizationInstance::CreateStatic(&FActionKeyFrameDetailsCustomizaiton::MakeInstance));

	UpdateActionKeyDetail();
	UpdateActionEventDetail();

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding( 3.0f, 2.0f )
		[
			KeyPropertyView->GetWidget().ToSharedRef()
		]
	];
}

TSharedRef<IPropertyTypeCustomization> SActionKeyDetailTab::CreateKeyNotifyCustomizationLayout()
{
	return MakeShared<FActionKeyNotifyCustomization>();
}

void SActionKeyDetailTab::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged)
{
	if (EditingSkill.IsValid())
	{
		UStruct* owner = PropertyThatChanged->GetOwnerStruct();
		if (owner->GetFName() == "ActionKeyFrame")
		{
			EditingSkill.Pin()->MarkDirty(true, PropertyThatChanged);
		}
	}

	if (EditingEvent.IsValid())
	{
		EditingEvent.Pin()->MarkDirty(true, PropertyThatChanged);
	}
}

void SActionKeyDetailTab::OnEditingKeyFrameChanged(FActionKeyFrame* NewKeyFrame)
{
	UpdateActionKeyDetail();
}

void SActionKeyDetailTab::OnEditingEventChanged(TWeakPtr<FActionEvent> NewEventPtr)
{
	UpdateActionEventDetail();
}

void SActionKeyDetailTab::UpdateActionKeyDetail()
{
	EditingSkill = ActionSkillEditor.Pin()->GetEditingSkill();

	FActionKeyFrame* KeyFrame = ActionSkillEditor.Pin()->GetEditingActionKeyFrame();
	if (KeyFrame)
	{
		EditingEvent = KeyFrame->GetEvent();
		KeyPropertyView->SetStructureData(MakeShareable(new FStructOnScope(FActionKeyFrame::StaticStruct(), (uint8*)KeyFrame)));
	}
	else
	{
		KeyPropertyView->SetStructureData(nullptr);
	}
}

void SActionKeyDetailTab::UpdateActionEventDetail()
{
	EditingEvent = ActionSkillEditor.Pin()->GetEditingEvent();

	if (EditingEvent.IsValid())
	{
		KeyPropertyView->SetStructureData(MakeShareable(new FStructOnScope(FActionEvent::StaticStruct(), (uint8*)EditingEvent.Pin().Get())));
	}
	else
	{
		KeyPropertyView->SetStructureData(nullptr);
	}
}
#undef LOCTEXT_NAMESPACE