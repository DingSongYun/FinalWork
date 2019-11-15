#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lua/ToLuaInterface.h"
#include "Engine/DataTable.h"
//#include "GameFramework/Character.h"
#include "WorkshopEditorTool.generated.h"

UCLASS()
class ATELIEREDITOR_API AWorkshopEditorTool : public AActor, public IToLuaInterface
{
	GENERATED_BODY()
	GENERATED_LUAINTERFACE_BODY()

public:
	// Sets default values for this actor's properties
	AWorkshopEditorTool();
	~AWorkshopEditorTool();

	void DestroyEditor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
