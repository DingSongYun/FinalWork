#include "ResizableBrushComponentVisualizer.h"
#include "Components/BrushComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Brush.h"
#include "ResizableVerticeBoxBuilder.h"

#include "SceneManagement.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "ActorEditorUtils.h"
#include "EditorStyleSet.h"

#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/Commands.h"

IMPLEMENT_HIT_PROXY(HBrushLineHandleProxy, HComponentVisProxy);
#define LOCTEXT_NAMESPACE "ResizableBrushComponentVisualizer"

/** Define commands for the spline component visualizer */
class FResizableBrushComponentVisualizerCommands : public TCommands<FResizableBrushComponentVisualizerCommands>
{
public:
	FResizableBrushComponentVisualizerCommands() : TCommands <FResizableBrushComponentVisualizerCommands>
	(
		"SplineComponentVisualizer",	// Context name for fast lookup
		LOCTEXT("SplineComponentVisualizer", "Spline Component Visualizer"),	// Localized context name for displaying
		NAME_None,	// Parent
		FEditorStyle::GetStyleSetName()
	)
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(DeleteKey, "Delete Spline Point", "Delete the currently selected spline point.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	}

public:
	/** Delete key */
	TSharedPtr<FUICommandInfo> DeleteKey;
};

FResizableBrushComponentVisualizer::FResizableBrushComponentVisualizer()
	: FComponentVisualizer()
	, bAllowDuplication(true)	
{
	FResizableBrushComponentVisualizerCommands::Register();
	
	ComponentVisualizerActions = MakeShareable(new FUICommandList);

	ResizableVerticeProperty = FindField<UProperty>(UResizableVerticeBoxBuilder::StaticClass(), GET_MEMBER_NAME_CHECKED(UResizableVerticeBoxBuilder, VerticeSet));
	BrushBuilderProperty = FindField<UProperty>(ABrush::StaticClass(), GET_MEMBER_NAME_CHECKED(ABrush, BrushBuilder));
}

FResizableBrushComponentVisualizer::~FResizableBrushComponentVisualizer()
{
	FResizableBrushComponentVisualizerCommands::Unregister();
}

void FResizableBrushComponentVisualizer::OnRegister()
{
	const auto& Commands = FResizableBrushComponentVisualizerCommands::Get();
	ComponentVisualizerActions->MapAction(
		Commands.DeleteKey,
		FExecuteAction::CreateSP(this, &FResizableBrushComponentVisualizer::DeleteSection),
		FCanExecuteAction::CreateSP(this, &FResizableBrushComponentVisualizer::CanDeleteSection)
	);
}

UBrushComponent* FResizableBrushComponentVisualizer::GetEditingComponent() const
{
	return Cast<UBrushComponent>(GetComponentFromPropertyName(EditingActor.Get(), EditingCompPropName));
}

UResizableVerticeBoxBuilder* FResizableBrushComponentVisualizer::GetEditingBrushBuilder() const
{
	if (ABrush* Brush = Cast<ABrush>(EditingActor))
	{
		return Cast<UResizableVerticeBoxBuilder>(Brush->BrushBuilder);
	}
	return nullptr;
}

void FResizableBrushComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{

	if (const UBrushComponent* BrushComponent = Cast<const UBrushComponent>(Component))
	{
		FTransform CompTM = BrushComponent->GetComponentTransform();
		if (ABrush* Brush = Cast<ABrush>(BrushComponent->GetOwner()))
		{
			if (UResizableVerticeBoxBuilder* BrushBuilder = Cast<UResizableVerticeBoxBuilder>(Brush->BrushBuilder))
			{
				//BrushBuilder.Z;
				for (int32 Index = 0; Index < BrushBuilder->VerticeSet.Num(); ++Index)
				{
					PDI->SetHitProxy(NULL);

					FVector& Vertice = BrushBuilder->VerticeSet[Index];
					FVector Start = CompTM.TransformPosition(FVector(Vertice.X, Vertice.Y, -1 * BrushBuilder->Z / 2));
					FVector Mid = CompTM.TransformPosition(FVector(Vertice.X, Vertice.Y, 0.f));
					FVector End = CompTM.TransformPosition(FVector(Vertice.X, Vertice.Y, 1 * BrushBuilder->Z / 2));

					PDI->SetHitProxy(new HBrushLineHandleProxy(Component,Index));
					auto Color = Index == EditingVerticeIndex ? BrushBuilder->EditorSelectedSegmentColor : BrushBuilder->EditorUnSelectedSegmentColor;
					PDI->DrawPoint(Mid, Color, 10.f, SDPG_Foreground);
					PDI->DrawLine(Start, End, Color, SDPG_Foreground, 2.f, 0.f);

					PDI->SetHitProxy(NULL);
				}
			}
		}
	}
}

bool FResizableBrushComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (VisProxy && VisProxy->Component.IsValid())
	{

		const UBrushComponent* BrushComp = CastChecked<const UBrushComponent>(VisProxy->Component.Get());

		EditingCompPropName = GetComponentPropertyName(BrushComp);
		if (EditingCompPropName.IsValid())
		{
			AActor* OldEditingActor = EditingActor.Get();
			EditingActor = BrushComp->GetOwner();
			if (EditingActor != OldEditingActor)
			{
				EditingVerticeIndex = INDEX_NONE;
			}

			if (VisProxy->IsA(HBrushLineHandleProxy::StaticGetType()))
			{
				HBrushLineHandleProxy* Proxy = (HBrushLineHandleProxy*)VisProxy;
				EditingVerticeIndex = Proxy->VerticeIndex;
			}
			return true;
		}
	}

	return false;
}

bool FResizableBrushComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UBrushComponent* Comp = GetEditingComponent();
	UResizableVerticeBoxBuilder* BrushBuilder = GetEditingBrushBuilder();
	if (BrushBuilder && Comp && EditingVerticeIndex != INDEX_NONE)
	{
		FTransform CompTM = Comp->GetComponentTransform();
		if (BrushBuilder->VerticeSet.IsValidIndex(EditingVerticeIndex))
		{
			OutLocation = CompTM.TransformPosition(BrushBuilder->VerticeSet[EditingVerticeIndex]);

			return true;
		}
	}
	return false;
}

void FResizableBrushComponentVisualizer::EndEditing()
{
	EditingActor = nullptr;
	EditingCompPropName.Clear();
	EditingVerticeIndex = INDEX_NONE;
}

bool FResizableBrushComponentVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	//if (GetEditingComponent())
	//{
	//	OutMatrix = FRotationMatrix::Make(FRotator::ZeroRotator);
	//	return true;
	//}
	return false;
}

bool FResizableBrushComponentVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;

	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		// Reset duplication flag on LMB release
		bAllowDuplication = true;
	}

	if (Event == IE_Pressed)
	{
		bHandled = ComponentVisualizerActions->ProcessCommandBindings(Key, FSlateApplication::Get().GetModifierKeys(), false);
	}

	return bHandled;
}

bool FResizableBrushComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	UBrushComponent* Comp = GetEditingComponent();
	UResizableVerticeBoxBuilder* BrushBuilder = GetEditingBrushBuilder();
	if (BrushBuilder && Comp && EditingVerticeIndex != INDEX_NONE)
	{
		FTransform CompTM = Comp->GetComponentTransform();

		
		if (ViewportClient->IsAltPressed() && bAllowDuplication)
		{
			DuplicateSection();

			// Don't duplicate again until we release LMB
			bAllowDuplication = false;
		}

		if (BrushBuilder->VerticeSet.IsValidIndex(EditingVerticeIndex))
		{
			FVector DeltaLocalTranslate = CompTM.InverseTransformVector(DeltaTranslate);
			BrushBuilder->VerticeSet[EditingVerticeIndex] += DeltaLocalTranslate;
		}

		NotifyPropertyChanged();

	}
	return true;
}

void FResizableBrushComponentVisualizer::NotifyPropertyChanged()
{

	auto Comp = GetEditingComponent();
	UResizableVerticeBoxBuilder* BrushBuilder = GetEditingBrushBuilder();
	check(Comp && BrushBuilder);

	AActor* Owner = Comp->GetOwner();
	FPropertyChangedEvent PropertyChangedEvent(ResizableVerticeProperty);

	//NotifyPropertyModified(Comp, ResizableVerticeProperty);

	BrushBuilder->PostEditChangeProperty(PropertyChangedEvent);

	FPropertyChangedEvent BrushBuilderPropertyChangedEvent(BrushBuilderProperty);
	Owner->PostEditChangeProperty(BrushBuilderPropertyChangedEvent);

	//Owner->PostEditMove(false);

}

void FResizableBrushComponentVisualizer::DeleteSection()
{
	UResizableVerticeBoxBuilder* BrushBuilder = GetEditingBrushBuilder();
	if (BrushBuilder && EditingVerticeIndex != INDEX_NONE)
	{
		if (BrushBuilder->VerticeSet.IsValidIndex(EditingVerticeIndex))
		{
			BrushBuilder->VerticeSet.RemoveAt(EditingVerticeIndex);
		}
	}
	NotifyPropertyChanged();
	EditingVerticeIndex = INDEX_NONE;
	GEditor->RedrawLevelEditingViewports(true);
}

void FResizableBrushComponentVisualizer::DuplicateSection()
{
	UResizableVerticeBoxBuilder* BrushBuilder = GetEditingBrushBuilder();
	if (BrushBuilder && EditingVerticeIndex != INDEX_NONE)
	{
		if (BrushBuilder->VerticeSet.IsValidIndex(EditingVerticeIndex))
		{
			EditingVerticeIndex = BrushBuilder->VerticeSet.Insert(FVector(BrushBuilder->VerticeSet[EditingVerticeIndex]), EditingVerticeIndex + 1);
		}
	}
	NotifyPropertyChanged();
	GEditor->RedrawLevelEditingViewports(true);
}

bool FResizableBrushComponentVisualizer::CanDeleteSection() const
{
	return EditingVerticeIndex != INDEX_NONE;
}

bool FResizableBrushComponentVisualizer::CanDuplicateSection() const
{
	return bAllowDuplication;
}

#undef LOCTEXT_NAMESPACE
