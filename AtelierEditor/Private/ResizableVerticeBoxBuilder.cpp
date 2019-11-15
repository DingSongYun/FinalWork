#include "ResizableVerticeBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"

#include "Engine/Brush.h"
#include "Engine/Polys.h"
#include "Editor.h"
#include "BSPOps.h"
#include "SnappingUtils.h"
#include "Engine/Selection.h"
#include "ObjectEditorUtils.h"

#define LOCTEXT_NAMESPACE "ResizableVerticeBox"

const FName GROUP_NAME = "ResizableVerticeBox";

UResizableVerticeBoxBuilder::UResizableVerticeBoxBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EditorSelectedSegmentColor(FLinearColor(1.0f, 0.0f, 0.0f))
	, EditorUnSelectedSegmentColor(FLinearColor(1.0f, 1.0f, 1.0f))
{
	// Base BrushBuilder Setting
	BitmapFilename = TEXT("Btn_Cylinder");
	ToolTip = TEXT("BrushBuilderName_ResizableVerticeBox");

	// Initialize Settings
	Z = 200.0f;
	VerticeSet.Add(FVector(50, 0, 0));
	VerticeSet.Add(FVector(-50, -50, 0));
	VerticeSet.Add(FVector(-50, 50, 0));

}

void UResizableVerticeBoxBuilder::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	//TODO: no need here
	if (!GIsTransacting)
	{
		// Rebuild brush on property change
		ABrush* Brush = Cast<ABrush>(GetOuter());
		if (Brush)
		{
			Brush->bInManipulation = PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive;
			Build(Brush->GetWorld(), Brush);
		}
	}

	bForceRefresh = false;
}

bool UResizableVerticeBoxBuilder::Build(UWorld* InWorld, ABrush* InBrush)
{
	int32 VerticeNum = VerticeSet.Num();

	if (VerticeNum < 3)
		return BadParameters(LOCTEXT("ResizableVerticeBoxSides", "Not enough Box sides(>= 3)."));

	if (Z <= 0)
		return BadParameters(LOCTEXT("ResizableVerticeBoxHeight", "Invalid Box Height(> 0)."));

	int32 Sides = VerticeSet.Num();

	BeginBrush(false, GROUP_NAME);
	BuildResizableBox(Sides);
	for( int32 j=-1; j<2; j+=2 )
	{
		PolyBegin( j, FName(TEXT("Cap")) );
		for( int32 i=0; i<Sides; i++ )
			Polyi( i*2+(1-j)/2 );
		PolyEnd();
	}

	return EndBrush(InWorld, InBrush);
}

void UResizableVerticeBoxBuilder::BuildResizableBox(int32 Sides)
{
	int32 n = GetVertexCount();
	int32 HalfHeight = Z / 2;

	for (int32 i = 0; i < Sides; i++)
	{
		FVector& Vertice = VerticeSet[i];
		Vertex3f(Vertice.X, Vertice.Y, -1 * HalfHeight);
		Vertex3f(Vertice.X, Vertice.Y, 1 * HalfHeight);
	}

	// Polys.
	for( int32 i=0; i< Sides; i++ )
		Poly4i( +1, n+i*2, n+i*2+1, n+((i*2+3)%(2*Sides)), n+((i*2+2)%(2*Sides)), FName(TEXT("Wall")) );
}

/*********************************************************************/
/*********************************************************************/
void bspValidateBrush( UModel* Brush, bool ForceValidate, bool DoStatusUpdate )
{
	check(Brush != nullptr);
	Brush->Modify();
	if( ForceValidate || !Brush->Linked )
	{
		Brush->Linked = 1;
		for( int32 i=0; i<Brush->Polys->Element.Num(); i++ )
		{
			Brush->Polys->Element[i].iLink = i;
		}
		int32 n=0;
		for( int32 i=0; i<Brush->Polys->Element.Num(); i++ )
		{
			FPoly* EdPoly = &Brush->Polys->Element[i];
			if( EdPoly->iLink==i )
			{
				for( int32 j=i+1; j<Brush->Polys->Element.Num(); j++ )
				{
					FPoly* OtherPoly = &Brush->Polys->Element[j];
					if
					(	OtherPoly->iLink == j
					&&	OtherPoly->Material == EdPoly->Material
					&&	OtherPoly->TextureU == EdPoly->TextureU
					&&	OtherPoly->TextureV == EdPoly->TextureV
					&&	OtherPoly->PolyFlags == EdPoly->PolyFlags
					&&	(OtherPoly->Normal | EdPoly->Normal)>0.9999 )
					{
						float Dist = FVector::PointPlaneDist( OtherPoly->Vertices[0], EdPoly->Vertices[0], EdPoly->Normal );
						if( Dist>-0.001 && Dist<0.001 )
						{
							OtherPoly->iLink = i;
							n++;
						}
					}
				}
			}
		}
// 		UE_LOG(LogBSPOps, Log,  TEXT("BspValidateBrush linked %i of %i polys"), n, Brush->Polys->Element.Num() );
	}

	// Build bounds.
	Brush->BuildBound();
}
void UResizableVerticeBoxBuilder::BeginBrush(bool InMergeCoplanars, FName InLayer)
{
	Layer = InLayer;
	MergeCoplanars = InMergeCoplanars;
	Vertices.Empty();
	Polys.Empty();
}

bool UResizableVerticeBoxBuilder::EndBrush(UWorld* InWorld, ABrush* InBrush)
{
	//!!validate
	check(InWorld != nullptr);
	ABrush* BuilderBrush = (InBrush != nullptr) ? InBrush : InWorld->GetDefaultBrush();

	// Ensure the builder brush is unhidden.
	BuilderBrush->bHidden = false;
	BuilderBrush->bHiddenEdLayer = false;

	AActor* Actor = GEditor->GetSelectedActors()->GetTop<AActor>();
	FVector Location;
	if (InBrush == nullptr)
	{
		Location = Actor ? Actor->GetActorLocation() : BuilderBrush->GetActorLocation();
	}
	else
	{
		Location = InBrush->GetActorLocation();
	}

	UModel* Brush = BuilderBrush->Brush;
	if (Brush == nullptr)
	{
		return true;
	}

	Brush->Modify();
	BuilderBrush->Modify();

	FRotator Temp(0.0f, 0.0f, 0.0f);
	FSnappingUtils::SnapToBSPVertex(Location, FVector::ZeroVector, Temp);
	BuilderBrush->SetActorLocation(Location, false);
	BuilderBrush->SetPivotOffset(FVector::ZeroVector);

	// Try and maintain the materials assigned to the surfaces. 
	TArray<FPoly> CachedPolys;
	UMaterialInterface* CachedMaterial = nullptr;
	if (Brush->Polys->Element.Num() == Polys.Num())
	{
		// If the number of polygons match we assume its the same shape.
		CachedPolys.Append(Brush->Polys->Element);
	}
	else if (Brush->Polys->Element.Num() > 0)
	{
		// If the polygons have changed check if we only had one material before. 
		CachedMaterial = Brush->Polys->Element[0].Material;
		if (CachedMaterial != NULL)
		{
			for (auto Poly : Brush->Polys->Element)
			{
				if (CachedMaterial != Poly.Material)
				{
					CachedMaterial = NULL;
					break;
				}
			}
		}
	}

	// Clear existing polys.
	Brush->Polys->Element.Empty();

	const bool bUseCachedPolysMaterial = CachedPolys.Num() > 0;
	int32 CachedPolyIdx = 0;
	for (TArray<FBuilderPoly>::TIterator It(Polys); It; ++It)
	{
		if (It->Direction < 0)
		{
			for (int32 i = 0; i < It->VertexIndices.Num() / 2; i++)
			{
				Exchange(It->VertexIndices[i], It->VertexIndices.Last(i));
			}
		}

		FPoly Poly;
		Poly.Init();
		Poly.ItemName = It->ItemName;
		Poly.Base = Vertices[It->VertexIndices[0]];
		Poly.PolyFlags = It->PolyFlags;

		// Try and maintain the polygons material where possible
		Poly.Material = (bUseCachedPolysMaterial) ? CachedPolys[CachedPolyIdx++].Material : CachedMaterial;

		for (int32 j = 0; j < It->VertexIndices.Num(); j++)
		{
			new(Poly.Vertices) FVector(Vertices[It->VertexIndices[j]]);
		}
		if (Poly.Finalize(BuilderBrush, 1) == 0)
		{
			new(Brush->Polys->Element)FPoly(Poly);
		}
	}

	if (MergeCoplanars)
	{
		GEditor->bspMergeCoplanars(Brush, 0, 1);
		bspValidateBrush(Brush, 1, 1);
	}
	Brush->Linked = 1;
	bspValidateBrush(Brush, 0, 1);
	Brush->BuildBound();

	GEditor->RedrawLevelEditingViewports();
	GEditor->SetPivot(BuilderBrush->GetActorLocation(), false, true);

	BuilderBrush->ReregisterAllComponents();

	return true;
}

int32 UResizableVerticeBoxBuilder::GetVertexCount() const
{
	return Vertices.Num();
}

FVector UResizableVerticeBoxBuilder::GetVertex(int32 i) const
{
	return Vertices.IsValidIndex(i) ? Vertices[i] : FVector::ZeroVector;
}

int32 UResizableVerticeBoxBuilder::GetPolyCount() const
{
	return Polys.Num();
}

bool UResizableVerticeBoxBuilder::BadParameters(const FText& Msg)
{
	if ( NotifyBadParams )
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Msg"), Msg);
		FNotificationInfo Info( FText::Format( LOCTEXT( "BadParameters", "Bad parameters in brush builder\n{Msg}" ), Arguments ) );
		Info.bFireAndForget = true;
		Info.ExpireDuration = Msg.IsEmpty() ? 4.0f : 6.0f;
		Info.bUseLargeFont = Msg.IsEmpty();
		Info.Image = FEditorStyle::GetBrush(TEXT("MessageLog.Error"));
		FSlateNotificationManager::Get().AddNotification( Info );
	}
	return 0;
}

int32 UResizableVerticeBoxBuilder::Vertexv(FVector V)
{
	int32 Result = Vertices.Num();
	new(Vertices)FVector(V);

	return Result;
}

int32 UResizableVerticeBoxBuilder::Vertex3f(float inX, float inY, float inZ)
{
	int32 Result = Vertices.Num();
	new(Vertices)FVector(inX, inY, inZ);
	return Result;	
}

void UResizableVerticeBoxBuilder::Poly3i(int32 Direction, int32 i, int32 j, int32 k, FName ItemName, bool bIsTwoSidedNonSolid )
{
	new(Polys)FBuilderPoly;
	Polys.Last().Direction=Direction;
	Polys.Last().ItemName=ItemName;
	new(Polys.Last().VertexIndices)int32(i);
	new(Polys.Last().VertexIndices)int32(j);
	new(Polys.Last().VertexIndices)int32(k);
	Polys.Last().PolyFlags = PF_DefaultFlags | (bIsTwoSidedNonSolid ? (PF_TwoSided|PF_NotSolid) : 0);
}

void UResizableVerticeBoxBuilder::Poly4i(int32 Direction, int32 i, int32 j, int32 k, int32 l, FName ItemName, bool bIsTwoSidedNonSolid )
{
	new(Polys)FBuilderPoly;
	Polys.Last().Direction=Direction;
	Polys.Last().ItemName=ItemName;
	new(Polys.Last().VertexIndices)int32(i);
	new(Polys.Last().VertexIndices)int32(j);
	new(Polys.Last().VertexIndices)int32(k);
	new(Polys.Last().VertexIndices)int32(l);
	Polys.Last().PolyFlags = PF_DefaultFlags | (bIsTwoSidedNonSolid ? (PF_TwoSided|PF_NotSolid) : 0);
}

void UResizableVerticeBoxBuilder::PolyBegin(int32 Direction, FName ItemName)
{
	new(Polys)FBuilderPoly;
	Polys.Last().ItemName=ItemName;
	Polys.Last().Direction = Direction;
	Polys.Last().PolyFlags = PF_DefaultFlags;
}

void UResizableVerticeBoxBuilder::Polyi(int32 i)
{
	new(Polys.Last().VertexIndices)int32(i);
}

void UResizableVerticeBoxBuilder::PolyEnd()
{
}
#undef LOCTEXT_NAMESPACE
