// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-04-16

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Builders/EditorBrushBuilder.h"
#include "ResizableVerticeBoxBuilder.generated.h"

class ABrush;

class UResizableVerticeBoxBuilder;

UCLASS(autoexpandcategories=BrushSettings, EditInlineNew, meta=(DisplayName="ResizableVerticeBox"))
class ATELIEREDITOR_API UResizableVerticeBoxBuilder : public UBrushBuilder
{
public:
	GENERATED_BODY()

public:
	UResizableVerticeBoxBuilder(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	//~ Begin UBrushBuilder Interface
	virtual bool Build( UWorld* InWorld, ABrush* InBrush = NULL ) override;
	//~ End UBrushBuilder Interface

	// @todo document
	virtual void BuildResizableBox(int32 Sides);

	/*********************************************************************/
	/*********************************************************************/
	virtual void BeginBrush(bool InMergeCoplanars, FName InLayer) override;
	virtual bool EndBrush(UWorld* InWorld, ABrush* InBrush) override;

	virtual int32 GetVertexCount() const override;
	virtual FVector GetVertex( int32 i ) const override;
	virtual int32 GetPolyCount() const override;
	virtual bool BadParameters(const FText& msg);

	virtual int32 Vertexv( FVector v ) override;
	virtual int32 Vertex3f(float X, float Y, float Z);

	virtual void Poly3i( int32 Direction, int32 i, int32 j, int32 k, FName ItemName = NAME_None, bool bIsTwoSidedNonSolid = false ) override;
	virtual void Poly4i( int32 Direction, int32 i, int32 j, int32 k, int32 l, FName ItemName = NAME_None, bool bIsTwoSidedNonSolid = false ) override;
	virtual void PolyBegin( int32 Direction, FName ItemName = NAME_None ) override;
	virtual void Polyi( int32 i ) override;
	virtual void PolyEnd() override;


public:
	/** Distance from base to top of box */
	UPROPERTY(EditAnywhere, Category=BrushSettings, meta=(ClampMin = "0.000001"))
	float Z;

	UPROPERTY(EditAnywhere, Category = BrushSettings, BlueprintReadWrite, meta=(MakeEditWidget=""))
	TArray<FVector> VerticeSet;

	UPROPERTY(EditAnywhere, Category = BrushSettings, BlueprintReadWrite)
	FLinearColor EditorSelectedSegmentColor;

	UPROPERTY(EditAnywhere, Category = BrushSettings, BlueprintReadWrite)
	FLinearColor EditorUnSelectedSegmentColor;

	UPROPERTY(EditAnywhere, Category = BrushSettings, BlueprintReadWrite)
	bool bForceRefresh;
};
