// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-11-03

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "System/CameraManager.h"

#include "PIECameraManager.generated.h"

UCLASS(BlueprintType, Blueprintable, nonTransient)
class AOfflineCameraController : public ACameraManager
{
	GENERATED_UCLASS_BODY()
	//GENERATED_BODY()

public:
	virtual ~AOfflineCameraController() override;

	void SpawnCameraManager();
	void SpawnCameraActor();

	UFUNCTION(BlueprintCallable, Category=CameraConfig)
	void PostInitialize();

	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category=CameraConfig, meta=(DisplayName = "PostInitialize"))
	void BP_PostInitialize(bool FlagHiddenInEditor = true);

	UFUNCTION(BlueprintCallable, CallInEditor, Category=CameraConfig, meta = (DisplayName = "锁定到相机"))
	void PossessViewport();

	UFUNCTION(BlueprintCallable, CallInEditor, Category=CameraConfig, meta = (DisplayName = "解除锁定"))
	void UnpossessViewport();

	virtual void PostInitializeComponents() override;

	void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const;
	virtual FMinimalViewInfo GetCameraCachePOV() const;
	virtual float GetFOVAngle() const;

	// 更新相机信息
	void UpdateCamera(float DeltaTime) override;
	// 更新ViewInfo
	void UpdateViewInfo(struct FMinimalViewInfo& OutPOV, float DeltaTime);
	// 用新的ViewInfo来刷新相机
	void SetupViewport(const FMinimalViewInfo& InPOV);
private:
	void LockViewportToCamera(class FLevelEditorViewportClient* InViewportClient, class ACameraActor* Camera);

public:
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class ACameraActor* InspectorCamera;
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class ACameraActor* ThirdPlayerCamera;

	//UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	//class AOfflineCameraManager* CameraManager;

	struct FMinimalViewInfo POV;
};

