// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: JiananHuang
// Date: 2018-11-29

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimNodes/AnimNode_MultipleModifyBones.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "BonePose.h"
#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_MultipleModifyBones.generated.h"

class FCompilerResultsLog;
class USkeletalMeshComponent;
struct FAnimNode_MultipleModifyBones;
struct HActor;

UCLASS(meta=(Keywords = "Modify Transforms"))
class ATELIEREDITOR_API UAnimGraphNode_MultipleModifyBones : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_MultipleModifyBones Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	// End of UEdGraphNode interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	virtual void CreateOutputPins() override;
	virtual void ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex) override;
	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/** Are we currently showing this pin */
	bool IsPinShown(const FName PinName) const;

	// set literal value for FVector
	void SetDefaultValue(const FName InDefaultValueName, const FVector& InValue);
	// get literal value for vector
	void GetDefaultValue(const FName UpdateDefaultValueName, FVector& OutVec);

	void GetDefaultValue(const FName PropName, FRotator& OutValue)
	{
		FVector Value;
		GetDefaultValue(PropName, Value);
		OutValue.Pitch = Value.X;
		OutValue.Yaw = Value.Y;
		OutValue.Roll = Value.Z;
	}

	template<class ValueType>
	ValueType GetNodeValue(const FName PropName, const ValueType& CompileNodeValue)
	{
		if (IsPinShown(PropName))
		{
			ValueType Val;
			GetDefaultValue(PropName, Val);
			return Val;
		}
		return CompileNodeValue;
	}

	void SetDefaultValue(const FName PropName, const FRotator& InValue)
	{
		FVector VecValue(InValue.Pitch, InValue.Yaw, InValue.Roll);
		SetDefaultValue(PropName, VecValue);
	}

	template<class ValueType>
	void SetNodeValue(const FName PropName, ValueType& CompileNodeValue, const ValueType& InValue)
	{
		if (IsPinShown(PropName))
		{
			SetDefaultValue(PropName, InValue);
		}
		CompileNodeValue = InValue;
	}

protected:
	// UAnimGraphNode_Base interface
	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;
	virtual FEditorModeID GetEditorMode() const override;
	virtual void CopyNodeDataToPreviewNode(FAnimNode_Base* InPreviewNode) override;
	virtual void CopyPinDefaultsToNodeData(UEdGraphPin* InPin) override;
	// End of UAnimGraphNode_Base interface

	// UAnimGraphNode_SkeletalControlBase interface
	virtual FText GetControllerDescription() const;
	virtual const FAnimNode_MultipleModifyBones* GetNode() const { return &Node; }
	// End of UAnimGraphNode_SkeletalControlBase interface

private:
	/** Constructing FText strings can be costly, so we cache the node's title */
	FNodeTitleTextTable CachedNodeTitles;

	// storing current widget mode 
	int32 CurWidgetMode;
};

