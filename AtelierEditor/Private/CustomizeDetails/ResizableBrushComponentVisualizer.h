#pragma once

#include "CoreMinimal.h"
#include "HitProxies.h"
#include "InputCoreTypes.h"
#include "ComponentVisualizer.h"

class FUICommandList;

struct HBrushLineHandleProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

public:
	HBrushLineHandleProxy(const UActorComponent* InComponent, int32 Index)
	: HComponentVisProxy(InComponent, HPP_Wireframe)
	, VerticeIndex(Index)
	{}

	int32 VerticeIndex;
};

class FResizableBrushComponentVisualizer : public FComponentVisualizer
{

public:
	FResizableBrushComponentVisualizer();
	virtual ~FResizableBrushComponentVisualizer();

	//~ Begin FComponentVisualizer Interface
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	//virtual TSharedPtr<SWidget> GenerateContextMenu() const override;
	virtual bool IsVisualizingArchetype() const override { return false; }
	//~ End FComponentVisualizer Interface

protected:
	class UBrushComponent* GetEditingComponent() const ;
	class UResizableVerticeBoxBuilder* GetEditingBrushBuilder() const;

	void NotifyPropertyChanged();

	void DeleteSection();
	void DuplicateSection();
	bool CanDeleteSection() const;
	bool CanDuplicateSection() const;
private:
	TWeakObjectPtr<AActor> EditingActor;
	FPropertyNameAndIndex EditingCompPropName;
	int32 EditingVerticeIndex;

	UProperty* BrushBuilderProperty;
	UProperty* ResizableVerticeProperty;

	bool bAllowDuplication;

	TSharedPtr<FUICommandList> ComponentVisualizerActions;
};
