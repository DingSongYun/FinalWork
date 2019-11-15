// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphNode_MultipleModifyBones.h"
#include "UnrealWidget.h"
#include "AnimNodeEditModes.h"
#include "AnimationGraphSchema.h"
#include "Animation/AnimationSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet2/CompilerResultsLog.h"
#include "DetailLayoutBuilder.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "System/Project.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_MultipleModifyBones

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_MultipleModifyBones::UAnimGraphNode_MultipleModifyBones(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurWidgetMode = (int32)FWidget::WM_Rotate;
}

void UAnimGraphNode_MultipleModifyBones::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UAnimGraphNode_MultipleModifyBones::GetControllerDescription() const
{
	return LOCTEXT("MultipleTransformModifyBones", "Multiple Transform (Modify) Bones");
}

FString UAnimGraphNode_MultipleModifyBones::GetNodeCategory() const
{
	return TEXT("Skeletal Control Nodes");
}

FText UAnimGraphNode_MultipleModifyBones::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_MultipleModifyBones_Tooltip", "The Transform Bone node alters the transform - i.e. Translation, Rotation, or Scale - of the bones");
}

FText UAnimGraphNode_MultipleModifyBones::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		return GetControllerDescription();
	}
	// @TODO: the bone can be altered in the property editor, so we have to 
	//        choose to mark this dirty when that happens for this to properly work
	else //if (!CachedNodeTitles.IsTitleCached(TitleType, this))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ControllerDescription"), GetControllerDescription());

		// FText::Format() is slow, so we cache this to save on performance
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AnimGraphNode_MultipleModifyBones_ListTitle", "{ControllerDescription} - Bone: {BoneName}"), Args), this);
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AnimGraphNode_MultipleModifyBones_Title", "{ControllerDescription}\nBone: {BoneName}"), Args), this);
		}
	}
	return CachedNodeTitles[TitleType];
}

void UAnimGraphNode_MultipleModifyBones::CopyNodeDataToPreviewNode(FAnimNode_Base* InPreviewNode)
{
	FAnimNode_MultipleModifyBones* ModifyBone = static_cast<FAnimNode_MultipleModifyBones*>(InPreviewNode);

	// copies Pin values from the internal node to get data which are not compiled yet
	ModifyBone->TransformBonesInfo = Node.TransformBonesInfo;
}

FEditorModeID UAnimGraphNode_MultipleModifyBones::GetEditorMode() const
{
	return "AnimGraph.SkeletalControl.MultipleModifyBone";
}

void UAnimGraphNode_MultipleModifyBones::CopyPinDefaultsToNodeData(UEdGraphPin* InPin)
{
	UE_LOG(LogAtelier, Warning, TEXT("CopyPinDefaultsToNodeData: %s"), *InPin->GetName());
}

void UAnimGraphNode_MultipleModifyBones::CreateOutputPins()
{
	CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("Pose"));
}

/*
void UAnimGraphNode_MultipleModifyBones::ConvertToComponentSpaceTransform(const USkeletalMeshComponent* SkelComp, const FTransform & InTransform, FTransform & OutCSTransform, int32 BoneIndex, EBoneControlSpace Space) const
{
	USkeleton * Skeleton = SkelComp->SkeletalMesh->Skeleton;

	switch (Space)
	{
	case BCS_WorldSpace:
	{
		OutCSTransform = InTransform;
		OutCSTransform.SetToRelativeTransform(SkelComp->GetComponentTransform());
	}
	break;

	case BCS_ComponentSpace:
	{
		// Component Space, no change.
		OutCSTransform = InTransform;
	}
	break;

	case BCS_ParentBoneSpace:
		if (BoneIndex != INDEX_NONE)
		{
			const int32 ParentIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(BoneIndex);
			if (ParentIndex != INDEX_NONE)
			{
				const int32 MeshParentIndex = Skeleton->GetMeshBoneIndexFromSkeletonBoneIndex(SkelComp->SkeletalMesh, ParentIndex);
				if (MeshParentIndex != INDEX_NONE)
				{
					const FTransform ParentTM = SkelComp->GetBoneTransform(MeshParentIndex);
					OutCSTransform = InTransform * ParentTM;
				}
				else
				{
					OutCSTransform = InTransform;
				}
			}
		}
		break;

	case BCS_BoneSpace:
		if (BoneIndex != INDEX_NONE)
		{
			const int32 MeshBoneIndex = Skeleton->GetMeshBoneIndexFromSkeletonBoneIndex(SkelComp->SkeletalMesh, BoneIndex);
			if (MeshBoneIndex != INDEX_NONE)
			{
				const FTransform BoneTM = SkelComp->GetBoneTransform(MeshBoneIndex);
				OutCSTransform = InTransform * BoneTM;
			}
			else
			{
				OutCSTransform = InTransform;
			}
		}
		break;

	default:
		if (SkelComp->SkeletalMesh)
		{
			UE_LOG(LogAnimation, Warning, TEXT("ConvertToComponentSpaceTransform: Unknown BoneSpace %d  for Mesh: %s"), (uint8)Space, *SkelComp->SkeletalMesh->GetFName().ToString());
		}
		else
		{
			UE_LOG(LogAnimation, Warning, TEXT("ConvertToComponentSpaceTransform: Unknown BoneSpace %d  for Skeleton: %s"), (uint8)Space, *Skeleton->GetFName().ToString());
		}
		break;
	}
}


FVector UAnimGraphNode_MultipleModifyBones::ConvertCSVectorToBoneSpace(const USkeletalMeshComponent* SkelComp, FVector& InCSVector, FCSPose<FCompactHeapPose>& MeshBases, const FName& BoneName, const EBoneControlSpace Space)
{
	FVector OutVector = InCSVector;

	if (MeshBases.GetPose().IsValid())
	{
		const FMeshPoseBoneIndex MeshBoneIndex(SkelComp->GetBoneIndex(BoneName));
		const FCompactPoseBoneIndex BoneIndex = MeshBases.GetPose().GetBoneContainer().MakeCompactPoseIndex(MeshBoneIndex);

		switch (Space)
		{
			// World Space, no change in preview window
		case BCS_WorldSpace:
		case BCS_ComponentSpace:
			// Component Space, no change.
			break;

		case BCS_ParentBoneSpace:
		{
			const FCompactPoseBoneIndex ParentIndex = MeshBases.GetPose().GetParentBoneIndex(BoneIndex);
			if (ParentIndex != INDEX_NONE)
			{
				const FTransform& ParentTM = MeshBases.GetComponentSpaceTransform(ParentIndex);
				OutVector = ParentTM.InverseTransformVector(InCSVector);
			}
		}
		break;

		case BCS_BoneSpace:
		{
			const FTransform& BoneTM = MeshBases.GetComponentSpaceTransform(BoneIndex);
			OutVector = BoneTM.InverseTransformVector(InCSVector);
		}
		break;
		}
	}

	return OutVector;
}

FQuat UAnimGraphNode_MultipleModifyBones::ConvertCSRotationToBoneSpace(const USkeletalMeshComponent* SkelComp, FRotator& InCSRotator, FCSPose<FCompactHeapPose>& MeshBases, const FName& BoneName, const EBoneControlSpace Space)
{
	FQuat OutQuat = FQuat::Identity;

	if (MeshBases.GetPose().IsValid())
	{
		const FMeshPoseBoneIndex MeshBoneIndex(SkelComp->GetBoneIndex(BoneName));
		const FCompactPoseBoneIndex BoneIndex = MeshBases.GetPose().GetBoneContainer().MakeCompactPoseIndex(MeshBoneIndex);

		FVector RotAxis;
		float RotAngle;
		InCSRotator.Quaternion().ToAxisAndAngle(RotAxis, RotAngle);

		switch (Space)
		{
			// World Space, no change in preview window
		case BCS_WorldSpace:
		case BCS_ComponentSpace:
			// Component Space, no change.
			OutQuat = InCSRotator.Quaternion();
			break;

		case BCS_ParentBoneSpace:
		{
			const FCompactPoseBoneIndex ParentIndex = MeshBases.GetPose().GetParentBoneIndex(BoneIndex);
			if (ParentIndex != INDEX_NONE)
			{
				const FTransform& ParentTM = MeshBases.GetComponentSpaceTransform(ParentIndex);
				FTransform InverseParentTM = ParentTM.Inverse();
				//Calculate the new delta rotation
				FVector4 BoneSpaceAxis = InverseParentTM.TransformVector(RotAxis);
				FQuat DeltaQuat(BoneSpaceAxis, RotAngle);
				DeltaQuat.Normalize();
				OutQuat = DeltaQuat;
			}
		}
		break;

		case BCS_BoneSpace:
		{
			const FTransform& BoneTM = MeshBases.GetComponentSpaceTransform(BoneIndex);
			FTransform InverseBoneTM = BoneTM.Inverse();
			FVector4 BoneSpaceAxis = InverseBoneTM.TransformVector(RotAxis);
			//Calculate the new delta rotation
			FQuat DeltaQuat(BoneSpaceAxis, RotAngle);
			DeltaQuat.Normalize();
			OutQuat = DeltaQuat;
		}
		break;
		}
	}

	return OutQuat;
}

FVector UAnimGraphNode_MultipleModifyBones::ConvertWidgetLocation(const USkeletalMeshComponent* SkelComp, FCSPose<FCompactHeapPose>& MeshBases, const FName& BoneName, const FVector& Location, const EBoneControlSpace Space)
{
	FVector WidgetLoc = FVector::ZeroVector;

	if (MeshBases.GetPose().IsValid())
	{
		USkeleton * Skeleton = SkelComp->SkeletalMesh->Skeleton;
		const FMeshPoseBoneIndex MeshBoneIndex(SkelComp->GetBoneIndex(BoneName));
		const FCompactPoseBoneIndex CompactBoneIndex = MeshBases.GetPose().GetBoneContainer().MakeCompactPoseIndex(MeshBoneIndex);

		switch (Space)
		{
			// GetComponentTransform() must be Identity in preview window so same as ComponentSpace
		case BCS_WorldSpace:
		case BCS_ComponentSpace:
		{
			// Component Space, no change.
			WidgetLoc = Location;
		}
		break;

		case BCS_ParentBoneSpace:

			if (CompactBoneIndex != INDEX_NONE)
			{
				const FCompactPoseBoneIndex CompactParentIndex = MeshBases.GetPose().GetParentBoneIndex(CompactBoneIndex);
				if (CompactParentIndex != INDEX_NONE)
				{
					const FTransform& ParentTM = MeshBases.GetComponentSpaceTransform(CompactParentIndex);
					WidgetLoc = ParentTM.TransformPosition(Location);
				}
			}
			break;

		case BCS_BoneSpace:

			if (CompactBoneIndex != INDEX_NONE)
			{
				const FTransform& BoneTM = MeshBases.GetComponentSpaceTransform(CompactBoneIndex);
				WidgetLoc = BoneTM.TransformPosition(Location);
			}
		}
	}

	return WidgetLoc;
}
*/

void UAnimGraphNode_MultipleModifyBones::GetDefaultValue(const FName UpdateDefaultValueName, FVector& OutVec)
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->PinName == UpdateDefaultValueName)
		{
			if (GetSchema()->IsCurrentPinDefaultValid(Pin).IsEmpty())
			{
				FString DefaultString = Pin->GetDefaultAsString();

				// Existing nodes (from older versions) might have an empty default value string; in that case we just fall through and return the zero vector below (which is the default value in that case).
				if (!DefaultString.IsEmpty())
				{
					TArray<FString> ResultString;

					//Parse string to split its contents separated by ','
					DefaultString.TrimStartAndEndInline();
					DefaultString.ParseIntoArray(ResultString, TEXT(","), true);

					check(ResultString.Num() == 3);

					OutVec.Set(
						FCString::Atof(*ResultString[0]),
						FCString::Atof(*ResultString[1]),
						FCString::Atof(*ResultString[2])
					);
					return;
				}
			}
		}
	}
	OutVec = FVector::ZeroVector;
}

void UAnimGraphNode_MultipleModifyBones::SetDefaultValue(const FName UpdateDefaultValueName, const FVector& Value)
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->PinName == UpdateDefaultValueName)
		{
			if (GetSchema()->IsCurrentPinDefaultValid(Pin).IsEmpty())
			{
				FString Str = FString::Printf(TEXT("%.3f,%.3f,%.3f"), Value.X, Value.Y, Value.Z);
				if (Pin->DefaultValue != Str)
				{
					PreEditChange(nullptr);
					GetSchema()->TrySetDefaultValue(*Pin, Str);
					PostEditChange();
					break;
				}
			}
		}
	}
}

bool UAnimGraphNode_MultipleModifyBones::IsPinShown(const FName PinName) const
{
	for (const FOptionalPinFromProperty& Pin : ShowPinForProperties)
	{
		if (Pin.PropertyName == PinName)
		{
			return Pin.bShowPin;
		}
	}
	return false;
}

void UAnimGraphNode_MultipleModifyBones::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, ArrayIndex);

	if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, Alpha))
	{
		Pin->bHidden = (GetNode()->AlphaInputType != EAnimAlphaInputType::Float);

		if (!Pin->bHidden)
		{
			Pin->PinFriendlyName = GetNode()->AlphaScaleBias.GetFriendlyName(GetNode()->AlphaScaleBiasClamp.GetFriendlyName(Pin->PinFriendlyName));
		}
	}

	if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, bAlphaBoolEnabled))
	{
		Pin->bHidden = (GetNode()->AlphaInputType != EAnimAlphaInputType::Bool);
	}

	if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, AlphaCurveName))
	{
		Pin->bHidden = (GetNode()->AlphaInputType != EAnimAlphaInputType::Curve);

		if (!Pin->bHidden)
		{
			Pin->PinFriendlyName = GetNode()->AlphaScaleBiasClamp.GetFriendlyName(Pin->PinFriendlyName);
		}
	}
}

void UAnimGraphNode_MultipleModifyBones::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);

	// Reconstruct node to show updates to PinFriendlyNames.
	if ((PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, AlphaScaleBias))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, bMapRange))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputRange, Min))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputRange, Max))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, Scale))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, Bias))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, bClampResult))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, ClampMin))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, ClampMax))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, bInterpResult))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, InterpSpeedIncreasing))
		|| (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FInputScaleBiasClamp, InterpSpeedDecreasing)))
	{
		ReconstructNode();
	}

	if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, AlphaInputType))
	{
		FScopedTransaction Transaction(LOCTEXT("ChangeAlphaInputType", "Change Alpha Input Type"));
		Modify();

		const FAnimNode_MultipleModifyBones* SkelControlNode = GetNode();

		// Break links to pins going away
		for (int32 PinIndex = 0; PinIndex < Pins.Num(); ++PinIndex)
		{
			UEdGraphPin* Pin = Pins[PinIndex];
			if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, Alpha))
			{
				if (GetNode()->AlphaInputType != EAnimAlphaInputType::Float)
				{
					Pin->BreakAllPinLinks();
				}
			}
			else if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, bAlphaBoolEnabled))
			{
				if (GetNode()->AlphaInputType != EAnimAlphaInputType::Bool)
				{
					Pin->BreakAllPinLinks();
				}
			}
			else if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, AlphaCurveName))
			{
				if (GetNode()->AlphaInputType != EAnimAlphaInputType::Curve)
				{
					Pin->BreakAllPinLinks();
				}
			}
		}

		ReconstructNode();

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAnimGraphNode_MultipleModifyBones::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	Super::CustomizeDetails(DetailBuilder);

	const FAnimNode_MultipleModifyBones* SkelControlNode = GetNode();
	TSharedRef<IPropertyHandle> NodeHandle = DetailBuilder.GetProperty(FName(TEXT("Node")), GetClass());

	if (SkelControlNode->AlphaInputType != EAnimAlphaInputType::Bool)
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, bAlphaBoolEnabled)));
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaBoolBlend)));
	}

	if (SkelControlNode->AlphaInputType != EAnimAlphaInputType::Float)
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, Alpha)));
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaScaleBias)));
	}

	if (SkelControlNode->AlphaInputType != EAnimAlphaInputType::Curve)
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaCurveName)));
	}

	if ((SkelControlNode->AlphaInputType != EAnimAlphaInputType::Float)
		&& (SkelControlNode->AlphaInputType != EAnimAlphaInputType::Curve))
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaScaleBiasClamp)));
	}
}

void UAnimGraphNode_MultipleModifyBones::ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{
	if (UAnimationSettings::Get()->bEnablePerformanceLog)
	{
		const FAnimNode_MultipleModifyBones* tpNode = GetNode();
		if (tpNode && tpNode->LODThreshold < 0)
		{
			MessageLog.Warning(TEXT("@@ contains no LOD Threshold."), this);
		}
	}
}

#undef LOCTEXT_NAMESPACE
