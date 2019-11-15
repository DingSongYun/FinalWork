// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-01

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "ActionSkill.h"
#include "ActionSkillGraphSchema.generated.h"

USTRUCT()
struct ACTIONSKILLEDITOR_API FSkillGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	// Simple type info
	static FName StaticGetTypeId() {static FName Type("FSkillGraphSchemaAction"); return Type;}
	virtual FName GetTypeId() const override { return StaticGetTypeId(); } 

	FSkillGraphSchemaAction() 
		: FEdGraphSchemaAction()
	{}

	FSkillGraphSchemaAction(TSharedPtr<struct FActionSkillScope> InActionSkill);

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override
	{
		return nullptr;
	}
	//~ End FEdGraphSchemaAction Interface

	FText GetSkillCategory();
	FText GetSkillName();
private:
	void OnActionSkillBeDirty(bool bDirty);
public:
	/** Pointer to inner ActionSkill */
	TSharedPtr<struct FActionSkillScope> ActionSkillPtr;
};