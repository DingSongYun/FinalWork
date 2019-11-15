// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-01

#include "ActionSkillGraphSchema.h"
#include "ActionSkill.h"

#define LOCTEXT_NAMESPACE "ActionSkill"

FSkillGraphSchemaAction::FSkillGraphSchemaAction(TSharedPtr<struct FActionSkillScope> InActionSkill)
{
	check(InActionSkill.IsValid());

	ActionSkillPtr = InActionSkill;
	Grouping = 0;
	SectionID = 0;

	UpdateSearchData(GetSkillName(), GetSkillName(), GetSkillCategory(), FText());
	InActionSkill->OnSkillDataDirty.BindRaw(this, &FSkillGraphSchemaAction::OnActionSkillBeDirty);
}

FText FSkillGraphSchemaAction::GetSkillCategory()
{
	return FText::FromName("All");
}

FText FSkillGraphSchemaAction::GetSkillName()
{
	return FText::FromString(FName::NameToDisplayString(ActionSkillPtr->GetName() + "(" + FString::FromInt(ActionSkillPtr->GetId()) + (ActionSkillPtr->IsDirty() ? ") *" : ")"), false));
}

void FSkillGraphSchemaAction::OnActionSkillBeDirty(bool bDirty)
{
	UpdateSearchData(GetSkillName(), GetSkillName(), GetSkillCategory(), FText());
}
#undef LOCTEXT_NAMESPACE