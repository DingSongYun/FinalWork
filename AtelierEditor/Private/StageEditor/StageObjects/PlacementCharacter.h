// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-10-24
#pragma once

#include "CoreMinimal.h"
#include "IStagePlacement.h"
#include "GameFramework/Character.h"
#include "StageLocationNode.h"
#include "PlacementCharacter.generated.h"

USTRUCT(BlueprintType)
struct FConfData_Character
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta=(DisplayName="角色ID"))
    int32 Id;

    UPROPERTY(EditAnywhere, meta=(DisplayName="角色生成时间段"))
    FDateTime SpawnTime;

    UPROPERTY(EditAnywhere, meta=(DisplayName="单个角色复活时间"))
    float RetrieveTime;

    UPROPERTY(EditAnywhere, meta=(DisplayName="索敌范围"))
    int HuntArea;

    UPROPERTY(EditAnywhere, meta=(DisplayName="警戒范围"))
    int CautionArea;

    UPROPERTY(EditAnywhere, meta=(DisplayName="巡逻范围"))
    int PatrolArea;

    UPROPERTY(EditAnywhere, meta=(DisplayName="最小巡逻频率"))
    float MinPatrolFrequence;

    UPROPERTY(EditAnywhere, meta=(DisplayName="最大巡逻频率"))
    float MaxPatrolFrequence;

    UPROPERTY(EditAnywhere, meta=(DisplayName="回返距离"))
    int GoBackDistance;

    UPROPERTY(EditAnywhere, meta=(DisplayName="巡逻路径"))
    TArray<FStageLocationNode> PatrolPath;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "巡逻行为ID"))
        FString PerformPatrolName;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "是否显示血条"))
    bool isShowHPBar;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "是否不会死亡"))
    bool isUndeadGather;
};

UCLASS(BlueprintType)
class APlacementCharacterSpawner : public AActor, public IStagePlacement
{
    GENERATED_UCLASS_BODY()
    GENERATED_SERIALIZE_INFO(ConfData, EPlacementIdentifier::NPCs)
public:
    virtual void BeginDestroy() override;
	virtual void PostRegisterAllComponents() override;
	virtual void UnregisterAllComponents(bool bForReregister = false) override;
	virtual void MarkComponentsAsPendingKill() override;
    virtual bool ShouldTickIfViewportsOnly() const;

    void AssembleCharacter(uint32 CharacterId);
    void PostAssembleCharacter();

    //~ Begin: IStagePlacement Interface
    virtual void PostSerialize() override;
    //~ End: IStagePlacement Interface
private:
    void CreateChildCharacter();
    void DestroyChildCharacter();
    ACharacter* GetChildCharacter() { return ChildCharacter; }
    void OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh);

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual bool CanEditChange(const UProperty* InProperty) const override;
    void Tick(float DeltaTime) override;
    void DrawDebug(float DeltaTime);

private:
    UPROPERTY( EditAnywhere, Category = "Placement Configuration", meta = ( ShowOnlyInnerProperties ) )
    FConfData_Character ConfData;

	/** The class of Actor to spawn */
	UPROPERTY(BlueprintReadOnly, Category=ChildActor, meta=(OnlyPlaceable, AllowPrivateAccess="true"))
	TSubclassOf<ACharacter>	ChildCharacterClass;

	/** The actor that we spawned and own */
	UPROPERTY(BlueprintReadOnly, Category=ChildActor, TextExportTransient, NonPIEDuplicateTransient, meta=(AllowPrivateAccess="true"))
	ACharacter*	ChildCharacter;

    UPROPERTY(EditAnywhere, Category = "Placement Configuration", AdvancedDisplay, meta=(DisplayName="是否显示索敌范围(仅编辑器)"))
    bool bShowHuntArea;

    UPROPERTY(EditAnywhere, Category = "Placement Configuration", AdvancedDisplay, meta=(DisplayName="是否显示警戒范围(仅编辑器)"))
    bool bShowCautionArea;

    UPROPERTY(EditAnywhere, Category = "Placement Configuration", AdvancedDisplay, meta=(DisplayName="是否显示巡逻范围(仅编辑器)"))
    bool bShowPatrolArea;
};