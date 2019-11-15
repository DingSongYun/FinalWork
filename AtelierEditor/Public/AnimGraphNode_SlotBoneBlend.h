// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-10-29

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraph/Classes/AnimGraphNode_BlendListBase.h"
#include "AnimGraph/Classes/AnimGraphNode_Slot.h"
#include "AnimNodes/AnimNode_SlotBoneBlend.h"
#include "EdGraphNodeUtils.h"

#include "AnimGraphNode_SlotBoneBlend.generated.h"

UCLASS()
class ATELIEREDITOR_API UAnimGraphNode_SlotBoneBlend : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_SlotBoneBlend Node;

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface

	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;
	// End of UK2Node interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	// End of UAnimGraphNode_Base interface

private:
	/** 缓存Title, 节省构造Text的消耗 */
	FNodeTitleTextTable CachedNodeTitles;
};