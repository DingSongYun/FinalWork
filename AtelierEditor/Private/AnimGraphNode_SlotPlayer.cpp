 
#include "AnimGraphNode_SlotPlayer.h"

#define LOCTEXT_NAMESPACE "SlotPlayerNodes"

UAnimGraphNode_SlotPlayer::UAnimGraphNode_SlotPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UAnimGraphNode_SlotPlayer::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UAnimGraphNode_SlotPlayer::GetTooltipText() const
{
	return LOCTEXT("AnimSlotPlayerNode_Tooltip", "Plays animation from Slot, the animation can be any asset type.");
}

FText UAnimGraphNode_SlotPlayer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Node.SlotName == NAME_None || !HasValidBlueprint() )
	{
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			return LOCTEXT("SlotPlayerNodeListTitle_NoName", "SlotPlayer '(No slot name)'");
		}
		else
		{
			return LOCTEXT("SlotPlayerNodeTitle_NoName", "(No slot name)\nSlot Player");
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
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("SlotPlayerNodeListTitle", "Slot Player '{SlotName}'"), Args), this);
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("SlotPlayerNodeTitle", "Slot Player '{SlotName}'\nGroup '{GroupName}'"), Args), this);
		}
	}
	return CachedNodeTitles[TitleType];
}

FString UAnimGraphNode_SlotPlayer::GetNodeCategory() const
{
	return TEXT("Slot");
}

void UAnimGraphNode_SlotPlayer::BakeDataDuringCompilation(class FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	if (!GIsCookerLoadingPackage && AnimBlueprint->TargetSkeleton) // Don't modify skeleton during cook
	{
		AnimBlueprint->TargetSkeleton->RegisterSlotNode(Node.SlotName);
	}
}

#undef LOCTEXT_NAMESPACE