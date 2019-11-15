
#include "AnimGraphNode_BSSlot.h"

#define LOCTEXT_NAMESPACE "BSSlotNodes"

UAnimGraphNode_BSSlot::UAnimGraphNode_BSSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UAnimGraphNode_BSSlot::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UAnimGraphNode_BSSlot::GetTooltipText() const
{
	return LOCTEXT("AnimBSSlotNode_Tooltip", "Plays animation from code using BlendSpace");
}

FText UAnimGraphNode_BSSlot::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Node.SlotName == NAME_None || !HasValidBlueprint() )
	{
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			return LOCTEXT("BSSlotNodeListTitle_NoName", "BS Slot '(No slot name)'");
		}
		else
		{
			return LOCTEXT("BSSlotNodeTitle_NoName", "(No slot name)\nBS Slot");
		}
	}
	// @TODO: the bone can be altered in the property editor, so we have to 
	//        choose to mark this dirty when that happens for this to properly work
	else //if (!CachedNodeTitles.IsTitleCached(TitleType, this))
	{
		UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
		FName GroupName = (AnimBlueprint->TargetSkeleton) ? AnimBlueprint->TargetSkeleton->GetSlotGroupName(Node.SlotName) : FAnimSlotGroup::DefaultGroupName;

		FFormatNamedArguments Args;
		Args.Add(TEXT("SlotName"), FText::FromName(Node.SlotName));
		Args.Add(TEXT("GroupName"), FText::FromName(GroupName));

		// FText::Format() is slow, so we cache this to save on performance
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("BSSlotNodeListTitle", "BS Slot '{SlotName}'"), Args), this);
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("BSSlotNodeTitle", "BS Slot '{SlotName}'\nGroup '{GroupName}'"), Args), this);
		}
	}
	return CachedNodeTitles[TitleType];
}

FString UAnimGraphNode_BSSlot::GetNodeCategory() const
{
	return TEXT("BlendSpace");
}

void UAnimGraphNode_BSSlot::BakeDataDuringCompilation(class FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	if (!GIsCookerLoadingPackage && AnimBlueprint->TargetSkeleton) // Don't modify skeleton during cook
	{
		AnimBlueprint->TargetSkeleton->RegisterSlotNode(Node.SlotName);
	}
}

#undef LOCTEXT_NAMESPACE
