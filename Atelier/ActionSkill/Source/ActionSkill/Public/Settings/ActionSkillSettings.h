// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-01

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "ActionSkillSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "ActionSkill"))
class ACTIONSKILL_API UActionSkillSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()
public:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(EditAnywhere, config, Category = Editing)
	FFilePath 						SkillTablePath;

	UPROPERTY(EditAnywhere, config, Category = Editing)
	FFilePath 						EventTablePath;

	UPROPERTY(EditAnywhere, Category = Editing)
	UScriptStruct*		SkillDefinitionStruct;
	/** 
	 * UScriptStruct can not be stored throw config by default,
	 * so we use a SoftObjectPath to stored the setting
	 */
	UPROPERTY(BlueprintReadOnly, config, Category = Editing)
	FSoftObjectPath		SkillDefinitionStructConfig;
};
