// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: I-RUNG.YOU
// Date: 2019-01-24

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Stage/InteractionPoint.h"
#include "Model/InteractionObject.h"
#include "InteractionChar.generated.h"

class UStaticMesh;



USTRUCT(BlueprintType)
struct FStatusData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "主挂点"))
		TArray<AInteractionPoint*> MainSockets;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "次挂点"))
		TArray<AInteractionPoint*> SubSockets;
};


USTRUCT(BlueprintType, Blueprintable)
struct FInteractionCharData
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
		int32 FilteredIndex;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
		int32 interactionID;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true", DisplayName = "名称(仅编辑器)"))
		FString name = "NewItem";

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true", DisplayName = "最大交互距离"))
		float interactorDistance;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (DisplayName = "交互挂点"))
		TArray<AInteractionPoint*> Sockets;

	TArray<FSocketData> SocketDatas;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (DisplayName = "交互物状态"))
		TArray<FStatusData> Status;

};



UCLASS()
class ATELIEREDITOR_API AInteractionChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AInteractionChar();
	~AInteractionChar();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true", DisplayName = "模型(骨骼网格)"))
		USkeletalMesh* PreviewMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true", DisplayName = "交互物参数设置"))
		FInteractionCharData chatData;

	void SpawnSocket(FSocketData data);

	void InitSocketList(TSharedPtr<const FInteractionConfigObject> item);

	void RefreshSocketDataList();

	void Setup(TSharedPtr<const FInteractionConfigObject> item);

	void SetupModel(FString ModelName);

	void RemoveAllSockets();

	void Refresh();

	TSharedRef<FInteractionConfigObject> GetData();


private:

	USkeletalMeshComponent* SkeletalBodyComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	  

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
