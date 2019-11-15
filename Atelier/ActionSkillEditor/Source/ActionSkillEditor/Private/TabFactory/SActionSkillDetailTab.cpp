// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-27

#include "SActionSkillDetailTab.h"
#include "ActionSkillEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "IStructureDetailsView.h"
#include "ActionSkillEditor.h"
#include "ActionSkill.h"

#define LOCTEXT_NAMESPACE "SAnimationBlendSpaceGridWidget"

SActionSkillDetailTab::~SActionSkillDetailTab()
{
	// ActionSkillEditor.Pin()->OnEditingSkillChanged().RemoveAll(this);
}

void SActionSkillDetailTab::Construct(const FArguments& InArgs, TWeakPtr<class IActionSkillEditor> InEditorPtr)
{
	check(InEditorPtr.IsValid());

	ActionSkillEditor = InEditorPtr;
	ActionSkillEditor.Pin()->OnEditingSkillChanged().AddRaw(this, &SActionSkillDetailTab::OnEditingSkillChanged);

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//TSharedRef<class IStructureDetailsView> FPropertyEditorModule::CreateStructureDetailView(
	//const FDetailsViewArgs& DetailsViewArgs, const FStructureDetailsViewArgs& StructureDetailsViewArgs, 
	//TSharedPtr<FStructOnScope> StructData, const FText& CustomName)
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

	PropertyView = EditModule.CreateStructureDetailView(DetailsViewArgs, StructureViewArgs, nullptr, LOCTEXT("ActionSkill", "Skill Detail"));
	{
		IDetailsView* DetailsView = PropertyView->GetDetailsView();
		DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateSP(this, &SActionSkillDetailTab::HandleDetailsViewIsPropertyEditable));
		DetailsView->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &SActionSkillDetailTab::HandleDetailsViewVisibility)));
	}

	UpdateSkillDetail();

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding( 3.0f, 2.0f )
		[
			PropertyView->GetWidget().ToSharedRef()
		]
	];
}

void SActionSkillDetailTab::OnEditingSkillChanged(TWeakPtr<struct FActionSkillScope> NewSkill)
{
	UpdateSkillDetail();
}

void SActionSkillDetailTab::OnEditingEventChanged(TWeakPtr<struct FActionEvent> NewEvent)
{
	if (NewEvent.IsValid()) 
	{
		PropertyView->SetStructureData(nullptr);
	}
}

void SActionSkillDetailTab::UpdateSkillDetail()
{
	if (ActionSkillEditor.Pin()->GetEditingSkill().IsValid())
	{
		PropertyView->SetStructureData(ActionSkillEditor.Pin()->GetEditingSkill()->GetData());
	}
	else
	{
		PropertyView->SetStructureData(nullptr);
	}
}

bool SActionSkillDetailTab::HandleDetailsViewIsPropertyEditable() const
{
	return true;
}

EVisibility SActionSkillDetailTab::HandleDetailsViewVisibility() const
{
	if (ActionSkillEditor.Pin()->GetEditingSkill().IsValid())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}

void SActionSkillDetailTab::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged)
{
	auto EditingSkill = ActionSkillEditor.Pin()->GetEditingSkill();
	if (EditingSkill.IsValid())
	{
		EditingSkill->MarkDirty(true, PropertyThatChanged);
	}
}
#undef LOCTEXT_NAMESPACE