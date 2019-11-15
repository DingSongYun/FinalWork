// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-21

#include "SActionSkillViewport.h"
#include "UnrealEd.h"
#include "Framework/Application/SlateApplication.h"
#include "SActionSkillEdViewportToolbar.h"

/*********************************************************/
// SActionSkillViewport
// Viewport Widget
/*********************************************************/
void SActionSkillEdViewport::Construct(const FArguments& InArgs, const FActionSkillEdViewportArgs& InRequiredArgs)
{
	PreviewScenePtr = InRequiredArgs.PreviewScene;
	TabBodyPtr = InRequiredArgs.TabBody;
	AssetEditorToolkitPtr = InRequiredArgs.AssetEditorToolkit;
	ViewportIndex = InRequiredArgs.ViewportIndex;

	SEditorViewport::Construct(
		SEditorViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		.AddMetaData<FTagMetaData>(TEXT("Persona.Viewport"))
	);

	Client->VisibilityDelegate.BindSP(this, &SActionSkillEdViewport::IsVisible);
}
	
TSharedRef<FEditorViewportClient> SActionSkillEdViewport::MakeEditorViewportClient()
{
	// Create viewport client
	ViewportClient = MakeShareable(new FActionSkillEdViewportClient(PreviewScenePtr.Pin().ToSharedRef(), SharedThis(this), AssetEditorToolkitPtr.Pin().ToSharedRef(), ViewportIndex));
	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->SetViewLocation(FVector( 150.0f, 200.f, 100.0f ));
	ViewportClient->SetViewRotation(FRotator(-15.0f, -90.0f, 0));

	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SActionSkillEdViewport::MakeViewportToolbar()
{
	return SAssignNew(ViewportToolbar, SActionSkillEdViewportToolbar, SharedThis(this));
}

void SActionSkillEdViewport::OnFocusViewportToSelection()
{
}

void SActionSkillEdViewport::PostUndo(bool bSuccess)
{
	ViewportClient->Invalidate();
}

void SActionSkillEdViewport::PostRedo(bool bSuccess)
{
	ViewportClient->Invalidate();
}

void SActionSkillEdViewport::SetCameraFollowMode(ESkillViewportCameraFollowMode InCameraFollowMode, FName InBoneName)
{
	ViewportClient->SetCameraFollowMode(InCameraFollowMode, InBoneName);
}

bool SActionSkillEdViewport::IsCameraFollowEnabled(ESkillViewportCameraFollowMode InCameraFollowMode) const
{
	return ViewportClient->GetCameraFollowMode() != ESkillViewportCameraFollowMode::None;
}

FName SActionSkillEdViewport::GetCameraFollowBoneName() const
{
	return ViewportClient->GetCameraFollowBoneName();
}
/*********************************************************/
// SActionSkillEdViewportTab
// Tab Widget
/*********************************************************/
void SActionSkillEdViewportTab::Construct(const FArguments& InArgs, const TSharedRef<class FActionSkillEditorPreviewScene>& InPreviewScene, const TSharedRef<class FAssetEditorToolkit>& InAssetEditorToolkit, int32 InViewportIndex)
{
	FActionSkillEdViewportArgs ViewportArgs(InPreviewScene, SharedThis(this), InAssetEditorToolkit, InViewportIndex);
	ViewportWidget = SNew(SActionSkillEdViewport, ViewportArgs);
	ViewportClient = ViewportWidget->GetViewportClient();

	TSharedPtr<SVerticalBox> ViewportContainer = nullptr;
	this->ChildSlot
	[
		SAssignNew(ViewportContainer, SVerticalBox)

		// Build our toolbar level toolbar
		+SVerticalBox::Slot()
		.FillHeight(1)
		[
			SNew(SOverlay)

			// The viewport
			+SOverlay::Slot()
			[
				ViewportWidget.ToSharedRef()
			]

			+SOverlay::Slot()
			.Padding(8)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SAssignNew(ViewportNotificationsContainer, SVerticalBox)
			]
		]
	];
}

SActionSkillEdViewportTab::SActionSkillEdViewportTab()
{

}

SActionSkillEdViewportTab::~SActionSkillEdViewportTab()
{

}

TSharedRef<IPersonaViewportState> SActionSkillEdViewportTab::SaveState() const
{
	TSharedRef<FViewportSaveData> State = MakeShareable(new(FViewportSaveData));
	State->Save();
	return State;
}

void SActionSkillEdViewportTab::RestoreState(TSharedRef<IPersonaViewportState> InState)
{
	TSharedRef<FViewportSaveData> State = StaticCastSharedRef<FViewportSaveData>(InState);
	State->Restore();
}

FEditorViewportClient& SActionSkillEdViewportTab::GetViewportClient() const
{
	return *ViewportClient;
}

TSharedRef<IPinnedCommandList> SActionSkillEdViewportTab::GetPinnedCommandList() const
{
	return ViewportWidget->GetViewportToolbar()->GetPinnedCommandList().ToSharedRef();
}

TWeakPtr<SWidget> SActionSkillEdViewportTab::AddNotification(TAttribute<EMessageSeverity::Type> InSeverity, TAttribute<bool> InCanBeDismissed, const TSharedRef<SWidget>& InNotificationWidget)
{
	TSharedPtr<SBorder> ContainingWidget = nullptr;
	TWeakPtr<SWidget> WeakNotificationWidget = InNotificationWidget;

		auto GetVisibility = [WeakNotificationWidget]()
	{
		if(WeakNotificationWidget.IsValid())
		{
			return WeakNotificationWidget.Pin()->GetVisibility();
		}

		return EVisibility::Collapsed;
	};

	ViewportNotificationsContainer->AddSlot()
	.HAlign(HAlign_Right)
	.AutoHeight()
	[
		SAssignNew(ContainingWidget, SBorder)
		.Visibility_Lambda(GetVisibility)
	];

	return ContainingWidget;
}

void SActionSkillEdViewportTab::RemoveNotification(const TWeakPtr<SWidget>& InContainingWidget)
{
	if(InContainingWidget.IsValid())
	{
		ViewportNotificationsContainer->RemoveSlot(InContainingWidget.Pin().ToSharedRef());
	}
}

FReply SActionSkillEdViewportTab::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return FReply::Unhandled();
}