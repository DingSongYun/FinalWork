// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-10-29

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraph/Classes/AnimGraphNode_BlendListBase.h"
#include "AnimNodes/AnimNode_LayeredDiverseBoneBlend.h"

#include "AnimGraphNode_LayeredDiverseBoneBlend.generated.h"

UCLASS()
class ATELIEREDITOR_API UAnimGraphNode_LayeredDiverseBoneBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_LayeredDiverseBoneBlend Node;

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface

	// Adds a new pose pin
	//@TODO: Generalize this behavior (returning a list of actions/delegates maybe?)
	virtual void AddPinToBlendByFilter();
	virtual void RemovePinFromBlendByFilter(UEdGraphPin* Pin);

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

};