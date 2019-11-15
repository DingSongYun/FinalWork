#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lua/ToLuaInterface.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMeshActor.h"
//#include "GameFramework/Character.h"
#include "FurnitureEditorTool.generated.h"



UCLASS()
class ATELIEREDITOR_API AFurnitureEditorTool : public AActor, public IToLuaInterface, public TSharedFromThis<AFurnitureEditorTool>
{
	GENERATED_BODY()
	GENERATED_LUAINTERFACE_BODY()

public:
	// Sets default values for this actor's properties
	AFurnitureEditorTool();
	~AFurnitureEditorTool();

	void DestroyEditor();
	AStaticMeshActor* GenGridVolume(AActor*, USkeletalMesh*);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	TArray<TSharedPtr<AStaticMeshActor>> GridVolumes;
};
