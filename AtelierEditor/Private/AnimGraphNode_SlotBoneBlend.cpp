
#include "AnimGraphNode_SlotBoneBlend.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Animation/AnimInstanceProxy.h"

#include "GraphEditorActions.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_SlotBoneBlend

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_SlotBoneBlend::UAnimGraphNode_SlotBoneBlend(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Node.AddPose();
}

FLinearColor UAnimGraphNode_SlotBoneBlend::GetNodeTitleColor() const
{
	return FLinearColor::Blue;
}

FText UAnimGraphNode_SlotBoneBlend::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_SlotBoneBlend_Tooltip", "Blend per bone with slot animation");
}

FText UAnimGraphNode_SlotBoneBlend::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// 如果没有设定Slot, 那么显示SlotName为'None'
	if (Node.SlotName == NAME_None || !HasValidBlueprint() )
	{
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			return LOCTEXT("SlotNodeListTitle_NoName", "Blend per bone with slot '(No slot)'");
		}
		else
		{
			return LOCTEXT("SlotNodeTitle_NoName", "(No slot name)\nBlend per bone with slot");
		}
	}
	// 如果有设定Slot，显示SlotName以及Slot GroupName
	else
	{
		UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
		FName GroupName = (AnimBlueprint->TargetSkeleton) ? AnimBlueprint->TargetSkeleton->GetSlotGroupName(Node.SlotName) : FAnimSlotGroup::DefaultGroupName;

		FFormatNamedArguments Args;
		Args.Add(TEXT("SlotName"), FText::FromName(Node.SlotName));
		Args.Add(TEXT("GroupName"), FText::FromName(GroupName));

		// FText::Format() is slow, so we cache this to save on performance
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("SlotNodeListTitle", "Blend per bone with slot  '{SlotName}'"), Args), this);
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("SlotNodeTitle", "Blend per bone with slot '{SlotName}'\nSlot group '{GroupName}'"), Args), this);
		}
	}

	return CachedNodeTitles[TitleType];
}

FString UAnimGraphNode_SlotBoneBlend::GetNodeCategory() const
{
	return TEXT("Blends");
}

void UAnimGraphNode_SlotBoneBlend::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	//if (!Context.bIsDebugging)
	//{
	//	Context.MenuBuilder->BeginSection("AnimGraphNodeSlotBoneblend", LOCTEXT("SlotBoneBlend", "Slto Bone Blend"));
	//	{
	//		if (Context.Pin != NULL)
	//		{
	//			// we only do this for normal BlendList/BlendList by enum, BlendList by Bool doesn't support add/remove pins
	//			if (Context.Pin->Direction == EGPD_Input)
	//			{
	//				//@TODO: Only offer this option on arrayed pins
	//				Context.MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().RemoveBlendListPin);
	//			}
	//		}
	//		else
	//		{
	//			Context.MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().AddBlendListPin);
	//		}
	//	}
	//	Context.MenuBuilder->EndSection();
	//}
}

void UAnimGraphNode_SlotBoneBlend::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}
#undef LOCTEXT_NAMESPACE
