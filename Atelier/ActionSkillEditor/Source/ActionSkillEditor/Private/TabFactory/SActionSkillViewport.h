// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-21
#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "IPersonaViewport.h"
#include "EditorUndoClient.h"
#include "ActionSkillEditorViewportClient.h"

/*********************************************************/
// FViewportSaveData
/*********************************************************/
struct FViewportSaveData : public IPersonaViewportState
{
public:
	void Save() {};
	void Restore() {};
};

/*********************************************************/
// FActionSkillEdViewportArgs
// For FActionSkillEdViewport construct
/*********************************************************/
struct FActionSkillEdViewportArgs
{
public:
	FActionSkillEdViewportArgs(
		TSharedRef<class FActionSkillEditorPreviewScene> InPreviewScene, TSharedRef<class SActionSkillEdViewportTab> InTabBody,
		TSharedRef<class FAssetEditorToolkit> InAssetEditorToolkit, int32 InViewportIndex
	)
	: PreviewScene(InPreviewScene), TabBody(InTabBody)
	, AssetEditorToolkit(InAssetEditorToolkit), ViewportIndex(InViewportIndex)
	{}
	
	TSharedRef<class FActionSkillEditorPreviewScene> PreviewScene;
	TSharedRef<class SActionSkillEdViewportTab> TabBody;
	TSharedRef<class FAssetEditorToolkit> AssetEditorToolkit;
	int32 ViewportIndex;
};

/*********************************************************/
// SActionSkillEdViewport
// Viewport Widget
/*********************************************************/
class SActionSkillEdViewport : public SEditorViewport, public FEditorUndoClient
{
public:
	SLATE_BEGIN_ARGS( SActionSkillEdViewport ) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FActionSkillEdViewportArgs& InRequiredArgs);
		
	// Begin: SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void OnFocusViewportToSelection() override;
	// End: End of SEditorViewport interface

	// Begin: FEditorUndoClient interface
	virtual void PostUndo( bool bSuccess );
	virtual void PostRedo( bool bSuccess );
	// End: FEditorUndoClient interface

	/** Get the viewport toolbar widget */
	TSharedPtr<class SActionSkillEdViewportToolbar> GetViewportToolbar() const { return ViewportToolbar; }
	TWeakPtr<class FActionSkillEditorPreviewScene> GetPreviewScene() const { return PreviewScenePtr; }

	/** Setup the camera follow mode */
	void SetCameraFollowMode(ESkillViewportCameraFollowMode InCameraFollowMode, FName InBoneName);
	bool IsCameraFollowEnabled(ESkillViewportCameraFollowMode InCameraFollowMode) const;
	FName GetCameraFollowBoneName() const;
protected:
	// Viewport client
	TSharedPtr<class FActionSkillEdViewportClient> ViewportClient;

	// Viewport toolbar
	TSharedPtr<class SActionSkillEdViewportToolbar> ViewportToolbar;

	// Pointer to the compound widget that owns this viewport widget
	TWeakPtr<class SActionSkillEdViewportTab> TabBodyPtr;

	// The preview scene that we are viewing
	TWeakPtr<class FActionSkillEditorPreviewScene> PreviewScenePtr;

	// The asset editor we are embedded in
	TWeakPtr<class FAssetEditorToolkit> AssetEditorToolkitPtr;

	/** Viewport index (0-3) */
	int32 ViewportIndex;
};

/*********************************************************/
// SActionSkillEdViewportTab
// Tab Widget
/*********************************************************/
class SActionSkillEdViewportTab : public IPersonaViewport
{
public:
	SLATE_BEGIN_ARGS( SActionSkillEdViewportTab ) {};
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<class FActionSkillEditorPreviewScene>& InPreviewScene, const TSharedRef<class FAssetEditorToolkit>& InAssetEditorToolkit, int32 InViewportIndex);
	SActionSkillEdViewportTab();
	virtual ~SActionSkillEdViewportTab();

	/** Begin: IPersonaViewport interface */
	virtual TSharedRef<IPersonaViewportState> SaveState() const override;
	virtual void RestoreState(TSharedRef<IPersonaViewportState> InState) override;
	virtual FEditorViewportClient& GetViewportClient() const override;
	virtual TSharedRef<IPinnedCommandList> GetPinnedCommandList() const override;
	virtual TWeakPtr<SWidget> AddNotification(TAttribute<EMessageSeverity::Type> InSeverity, TAttribute<bool> InCanBeDismissed, const TSharedRef<SWidget>& InNotificationWidget) override;
	virtual void RemoveNotification(const TWeakPtr<SWidget>& InContainingWidget) override;
	/** End: IPersonaViewport interface */

	/** Begin: SWidget interface */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	/** End: SWidget interface */

private:
	/** Weak pointer back to the preview scene we are viewing */
	TWeakPtr<class FActionSkillEditorPreviewScene> PreviewScenePtr;

	/** Weak pointer back to asset editor we are embedded in */
	TWeakPtr<class FAssetEditorToolkit> FAssetEditorToolkit;

	/** Viewport widget */
	TSharedPtr<class SActionSkillEdViewport> ViewportWidget;

	// Viewport client
	TSharedPtr<class FEditorViewportClient> ViewportClient;


	/** Box that contains notifications */
	TSharedPtr<SVerticalBox> ViewportNotificationsContainer;
};