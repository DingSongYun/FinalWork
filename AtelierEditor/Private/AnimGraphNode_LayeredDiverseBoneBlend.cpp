
#include "AnimGraphNode_LayeredDiverseBoneBlend.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimNodes/AnimNode_LayeredDiverseBoneBlend.h"

#include "GraphEditorActions.h"
#include "ScopedTransaction.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_LayeredDiverseBoneBlend

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_LayeredDiverseBoneBlend::UAnimGraphNode_LayeredDiverseBoneBlend(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Node.AddPose();
}

FLinearColor UAnimGraphNode_LayeredDiverseBoneBlend::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.8f, 0.2f);
}

FText UAnimGraphNode_LayeredDiverseBoneBlend::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_LayeredDiverseBoneBlend_Tooltip", "Layered diverse blend per bone");
}

FText UAnimGraphNode_LayeredDiverseBoneBlend::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_LayeredDiverseBoneBlend_Title", "Layered diverse blend per bone");
}

FString UAnimGraphNode_LayeredDiverseBoneBlend::GetNodeCategory() const
{
	return TEXT("Blends");
}

void UAnimGraphNode_LayeredDiverseBoneBlend::AddPinToBlendByFilter()
{
	FScopedTransaction Transaction( LOCTEXT("AddPinToBlend", "AddPinToBlendByFilter") );
	Modify();

	Node.AddPose();
	ReconstructNode();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

void UAnimGraphNode_LayeredDiverseBoneBlend::RemovePinFromBlendByFilter(UEdGraphPin* Pin)
{
	FScopedTransaction Transaction( LOCTEXT("RemovePinFromBlend", "RemovePinFromBlendByFilter") );
	Modify();

	UProperty* AssociatedProperty;
	int32 ArrayIndex;
	GetPinAssociatedProperty(GetFNodeType(), Pin, /*out*/ AssociatedProperty, /*out*/ ArrayIndex);

	if (ArrayIndex != INDEX_NONE)
	{
		//@TODO: ANIMREFACTOR: Need to handle moving pins below up correctly
		// setting up removed pins info 
		RemovedPinArrayIndex = ArrayIndex;
		Node.RemovePose(ArrayIndex);
		ReconstructNode();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}
}

void UAnimGraphNode_LayeredDiverseBoneBlend::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	if (!Context.bIsDebugging)
	{
		Context.MenuBuilder->BeginSection("AnimGraphNodeLayeredDiverseBoneblend", LOCTEXT("LayeredDiverseBoneBlend", "Layered Diverse Bone Blend"));
		{
			if (Context.Pin != NULL)
			{
				// we only do this for normal BlendList/BlendList by enum, BlendList by Bool doesn't support add/remove pins
				if (Context.Pin->Direction == EGPD_Input)
				{
					//@TODO: Only offer this option on arrayed pins
					Context.MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().RemoveBlendListPin);
				}
			}
			else
			{
				Context.MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().AddBlendListPin);
			}
		}
		Context.MenuBuilder->EndSection();
	}
}

void UAnimGraphNode_LayeredDiverseBoneBlend::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Node.ValidateData();
}
#undef LOCTEXT_NAMESPACE
