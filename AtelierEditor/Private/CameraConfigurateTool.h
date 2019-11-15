// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-11-02

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraTween.h"
#include "Engine/DataTable.h"
#include "EditorUtilities.h"

#include "CameraConfigurateTool.generated.h"

UENUM(BlueprintType)
enum class EEditMode : uint8
{
	Preview,
	Edit_FocusMode,
	Edit_FreeMode
};

USTRUCT(BlueprintType)
struct FMultiCoordPOVParam
{
	GENERATED_USTRUCT_BODY()
public:

	/*Tween To Location*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera, meta = (DisplayName = "位置"))
	FVector Location;

	/*Tween To Rotation*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera, meta = (DisplayName = "旋转"))
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera, meta = (DisplayName = "FOV"))
	float FOV;

	UPROPERTY()
	FCameraPOVParam CachedParam;

	void RecalculateFromCachedParam(class AActor* Target, EEditMode Mode);
	void FilledToCachedParam(class AActor* Target, EEditMode Mode);
};

UCLASS(BlueprintType, Blueprintable)
class ATELIEREDITOR_API ACameraConfigurateTool: public AActor
{
	GENERATED_UCLASS_BODY()
public:
	virtual void BeginPlay() override;
	~ACameraConfigurateTool();
	virtual void Destroyed();

	UFUNCTION(CallInEditor, Category=CameraConfig, meta = (DisplayName = "初始化配置环境"))
	void InitConfigEnv()
	{
		return BP_InitConfigEnv(!IsPlaying());
	}

	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category=CameraConfig)
	void BP_InitConfigEnv(bool InDesignTime = true);

	UFUNCTION(CallInEditor, Category=CameraConfig, meta = (DisplayName = "播放镜头动画"))
	void PlayCameraMotion()
	{
		return BP_PlayCameraMotion();
	}

	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category=CameraConfig)
	void BP_PlayCameraMotion(bool FlagHiddenInEditor = true);

	UFUNCTION(CallInEditor, Category=CameraConfig, meta = (DisplayName = "重置镜头"))
	void ResetCamera()
	{
		return BP_ResetCamera();
	}
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category=CameraConfig)
	void BP_ResetCamera(bool FlagHiddenInEditor = true);

	UFUNCTION(BlueprintCallable, CallInEditor, Category=CameraConfig, meta = (DisplayName = "保存当前配置"))
	void DumpConfiguration()
	{
		return BP_DumpConfiguration();
	}

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, CallInEditor, Category=CameraConfig)
	void BP_DumpConfiguration(bool FlagHiddenInEditor = true);

	void PostLoadCameraConfiguration(FName inCameraID);

	UFUNCTION(BlueprintImplementableEvent, Category=CameraConfig, CallInEditor)
	void BP_PostLoadCameraConfiguration(FName inCameraID);

	void OnEditModeChanged(EEditMode Mode);

	UFUNCTION(BlueprintCallable, Category=CameraConfig)
	void SetActorLocked(class AActor* Actor);

	UFUNCTION(BlueprintCallable, Category=CameraConfig)
	void FlushLockedActor();
	void SyncLockedActor();

	/*Property Behaviors*/
	bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//virtual void PreEditChange( FEditPropertyChain& PropertyAboutToChange ) override;

	UFUNCTION(BlueprintCallable, Category=CameraConfigTool, CustomThunk, meta=(CustomStructureParam = "RowData"))
	bool DumpConfigurationNative(UDataTable* ToDataTable, FName RowName, FTableRowBase RowData);
	DECLARE_FUNCTION(execDumpConfigurationNative)
	{
        P_GET_OBJECT(UDataTable, ToDataTable);
        P_GET_PROPERTY(UNameProperty, RowName);

        //P_GET_OBJECT(UStructProperty, RowData);
		/* Get RowData */
        Stack.StepCompiledIn<UStructProperty>(NULL);
        void* RowData = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;
		{
			P_NATIVE_BEGIN;
			bSuccess = FEditorUtilities::DumpRowToDataTable(ToDataTable, RowName, RowData);
			P_NATIVE_END;
		}

		*(bool*)RESULT_PARAM = true;
	}

	UFUNCTION(BlueprintPure, Category=CameraConfigTool)
	bool IsPlaying();

	UFUNCTION(BlueprintCallable, Category=CameraConfigTool)
	void SetMultiCoordParam(const FMultiCoordPOVParam& MultiCoordParam, const FCameraPOVParam& Param)
	{
		FMultiCoordPOVParam* ParamPtr = const_cast<FMultiCoordPOVParam*>(&MultiCoordParam);
		ParamPtr->CachedParam = Param;
	}

	UFUNCTION(BlueprintPure, Category=CameraConfigTool)
	void GetMultiCoordParam(const FMultiCoordPOVParam& MultiCoordParam, FCameraPOVParam& Param)
	{
		FMultiCoordPOVParam* ParamPtr = const_cast<FMultiCoordPOVParam*>(&MultiCoordParam);
		Param = MultiCoordParam.CachedParam;
	}

	void RefreshMultiCoordParam();

private:
	void OnActorMove(AActor* InActor);
	void OnActorLockedMove(USceneComponent* InRootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
	bool IsEditMode() const;
	
	//FRotator LookRotationToTarget(const FVector& FromLocation, const AActor* Target) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, meta = (DisplayName = "编辑模式"))
	EEditMode Mode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, meta = (DisplayName = "镜头编号"))
	FName CameraID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, meta = (DisplayName = "镜头发起目标"))
	AActor* CamFromTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, meta = (DisplayName = "镜头聚焦目标"))
	AActor* CamFocusTarget;

	/** Begin: For Configuration */
	// 镜头移动开始使用当前相机，无需做切换
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "使用当前镜头作为动画初始位置"))
	bool bCameraStartUseCurrent;

	// 开启此选项时，镜头的ToParam会随着caller到target的距离在
	// CameraToParam 和 CameraToParamAdd中进行插值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "镜头目标位置随着距离进行插值"))
	bool bInterpolateByDistance;

	// 镜头开始移动时的参数，bCameraFromUseCurrent为false时起效果
	// 如果使用此参数，镜头需瞬切到当前参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "镜头初始参数"))
	FMultiCoordPOVParam CameraFromParam;

	// 镜头移动的终了参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "镜头动画参数"))
	FMultiCoordPOVParam CameraToParam;

	// 镜头移动的终了参数2,用来做随距离插值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "镜头动画参数2(插值用)"))
	FMultiCoordPOVParam CameraToParamAdd;
	
	// 进行插值时的基础距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "基础距离"))
	float BaseDistance;

	// 我们会定义每次的镜头动作皆有一个发起者(caller) 和 聚焦者(target)
	// 此参数用以确定是否需要将caller移动向target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "移动到目标位置"))
	bool bNeedMoveMeToTarget;

	// caller距离target的距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "需距离目标位置"))
	float DistanceMeToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "使用曲线"))
	bool bUseCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "曲线"))
	TSoftObjectPtr<class UCurveFloat> Curve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraConfig, AdvancedDisplay, meta = (DisplayName = "动画时长"))
	float Duration;
	
	/** End: For Configuration */
private:
	class AActor* ActorLocked;
};
