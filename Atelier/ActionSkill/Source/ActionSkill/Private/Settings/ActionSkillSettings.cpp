// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-01

#include "ActionSkillSettings.h"
#include "ActionSkill.h"

UActionSkillSettings::UActionSkillSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = TEXT("Atelier");
	SectionName = TEXT("ActionSkill");
	SkillTablePath.FilePath = TEXT("");
	EventTablePath.FilePath = TEXT("");
	SkillDefinitionStruct = FActionSkillTemplate::StaticStruct();
}

#if WITH_EDITOR
void UActionSkillSettings::PostInitProperties()
{
	Super::PostInitProperties();
	SkillDefinitionStruct = Cast<UScriptStruct>(SkillDefinitionStructConfig.TryLoad());
}

void UActionSkillSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UActionSkillSettings, SkillDefinitionStruct))
	{
		SkillDefinitionStructConfig = FSoftObjectPath(SkillDefinitionStruct);
	}
}
#endif