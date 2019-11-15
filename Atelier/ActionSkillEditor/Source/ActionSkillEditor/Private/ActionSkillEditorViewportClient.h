// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "Toolkits/AssetEditorToolkit.h"

UENUM()
enum class ESkillViewportCameraFollowMode : uint8
{
	/** Standard camera controls */
	None,

	/** Follow the bounds of the mesh */
	Bounds,

	/** Follow a bone or socket */
	Bone,
};

class FActionSkillEdViewportClient: public FEditorViewportClient
{
public:
	FActionSkillEdViewportClient(
		const TSharedRef<class FActionSkillEditorPreviewScene>& InPreviewScene, const TSharedRef<class SActionSkillEdViewport>& InViewport,
		const TSharedRef<class FAssetEditorToolkit>& InAssetEditorToolkit, int32 InViewportIndex
	);
	virtual ~FActionSkillEdViewportClient();

	void SetCameraFollowMode(ESkillViewportCameraFollowMode InCameraFollowMode, FName InBoneName);
	ESkillViewportCameraFollowMode GetCameraFollowMode() const { return CameraFollowMode; }
	FName GetCameraFollowBoneName() const { return CameraFollowBoneName; }

	virtual void Tick(float DeltaSeconds) override;

private:
	void HandlerPreviewScenePreTick();
	void HandlerPreviewScenePostTick();

private:
	// The preview scene that we are viewing
	TWeakPtr<class FActionSkillEditorPreviewScene> PreviewScenePtr;

	// The asset editor we are embedded in
	TWeakPtr<class FAssetEditorToolkit> AssetEditorToolkitPtr;

	/** Viewport index (0-3) */
	int32 ViewportIndex;

	/** The current camera follow mode */
	ESkillViewportCameraFollowMode CameraFollowMode;

	/** The bone we will follow when in EAnimationViewportCameraFollowMode::Bone */
	FName CameraFollowBoneName;

	/** Relative view location stored to match it pre/post tick */
	FVector RelativeViewLocation;

	/** When orbiting, the base rotation of the camera, allowing orbiting around different axes to Z */
	FQuat OrbitRotation;
};