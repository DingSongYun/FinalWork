// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24
#include "SActionSkillEdViewportToolbar.h"
#include "EditorStyleSet.h"
#include "IPinnedCommandList.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "EditorStyleSet.h"
#include "Widgets/SBoxPanel.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SEditorViewportViewMenu.h"
#include "TabFactory/SActionSkillViewport.h"
#include "ActionSkillEditorPreviewScene.h"
#include "EditorViewportCommands.h"
#include "BoneSelectionWidget.h"
#include "GameFramework/Character.h"
#include "ActionSkillEditorViewportClient.h"
#include "Components/SkeletalMeshComponent.h"

#define LOCTEXT_NAMESPACE "ActionSkillEditorViewportViewMenu"

static const FName DefaultForegroundName("DefaultForeground");
static const FMargin ToolbarSlotPadding(2.0f, 2.0f);
static const FMargin ToolbarButtonPadding(2.0f, 0.0f);

void SActionSkillEdViewportToolbar::Construct(const FArguments& InArgs, const TSharedRef<class SActionSkillEdViewport> InViewport)
{
	Viewport = InViewport;

	// Create our pinned commands before we bind commands
	IPinnedCommandListModule& PinnedCommandListModule = FModuleManager::LoadModuleChecked<IPinnedCommandListModule>(TEXT("PinnedCommandList"));
	PinnedCommands = PinnedCommandListModule.CreatePinnedCommandList( TEXT("ActionSkillViewport"));
	PinnedCommands->SetStyle(&FEditorStyle::Get(), TEXT("ViewportPinnedCommandList"));

	Extenders = InArgs._Extenders;
	// Extenders.Add(GetViewMenuExtender(InViewport));

	// If we have no extender, make an empty one
	if (Extenders.Num() == 0)
	{
		Extenders.Add(MakeShared<FExtender>());
	}

	TSharedRef<SHorizontalBox> LeftToolbar = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportToolbarMenu)
			.ToolTipText(LOCTEXT("ViewMenuTooltip", "View Options.\nShift-clicking items will 'pin' them to the toolbar."))
			.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Image("EditorViewportToolBar.MenuDropdown")
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.MenuDropdown")))
			.OnGetMenuContent(this, &SActionSkillEdViewportToolbar::GenerateViewMenu)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportViewMenu, InViewport, SharedThis(this))
			.ToolTipText(LOCTEXT("ViewModeMenuTooltip", "View Mode Options. Use this to change how the view is rendered, e.g. Lit/Unlit."))
			.MenuExtenders(FExtender::Combine(Extenders))
		];

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( FEditorStyle::GetBrush("NoBorder") )
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		.ColorAndOpacity( this, &SViewportToolBar::OnGetColorAndOpacity )
		.ForegroundColor( FEditorStyle::GetSlateColor(DefaultForegroundName) )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				LeftToolbar
			]
		]
	];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SActionSkillEdViewportToolbar::GenerateViewMenu() const
{
	TSharedPtr<FExtender> MenuExtender = FExtender::Combine(Extenders);

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder InMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList(), MenuExtender);
	InMenuBuilder.PushExtender(MenuExtender.ToSharedRef());
		InMenuBuilder.BeginSection("AnimViewportCamera", LOCTEXT("ViewMenu_CameraLabel", "Camera"));
	{
		InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().FocusViewportToSelection);

		InMenuBuilder.AddSubMenu(
			LOCTEXT("CameraFollowModeLabel", "Camera Follow Mode"),
			LOCTEXT("CameraFollowModeTooltip", "Set various camera follow modes"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InSubMenuBuilder)
			{
				InSubMenuBuilder.BeginSection("SkillViewportCameraFollowMode", LOCTEXT("ViewMenu_CameraFollowModeLabel", "Camera Follow Mode"));
				{
					InSubMenuBuilder.PushCommandList(Viewport.Pin()->GetCommandList().ToSharedRef());

					InSubMenuBuilder.AddMenuEntry(
						FText::FromString("Camera Follow None"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateLambda([this] () {
							Viewport.Pin()->SetCameraFollowMode(ESkillViewportCameraFollowMode::None, NAME_None);
						}))
					);
					InSubMenuBuilder.AddMenuEntry(
						FText::FromString("Camera Follow Bounds"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateLambda([this] () {
							Viewport.Pin()->SetCameraFollowMode(ESkillViewportCameraFollowMode::Bounds, NAME_None);
						}))
					);
					InSubMenuBuilder.AddMenuEntry(
						FText::FromString("Camera Follow Skill"), 
						FText::FromString(""), 
						FSlateIcon(), 
						FUIAction(FExecuteAction::CreateLambda([this] () {
							Viewport.Pin()->SetCameraFollowMode(ESkillViewportCameraFollowMode::Bone, "Point_Camera001_Root");
						}))
					);

					InSubMenuBuilder.PopCommandList();
				}
				InSubMenuBuilder.EndSection();
		
				InSubMenuBuilder.BeginSection("SkillViewportCameraFollowBone", FText());
				{
					InSubMenuBuilder.AddWidget(MakeFollowBoneWidget(), FText(), true);
				}
				InSubMenuBuilder.EndSection();
			}),
			false,
			FSlateIcon(FEditorStyle::GetStyleSetName(), "SkillViewportMenu.CameraFollow")
			);

		InMenuBuilder.AddWidget(MakeFOVWidget(), LOCTEXT("Viewport_FOVLabel", "Field Of View"));
	}
	InMenuBuilder.EndSection();

	return InMenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SActionSkillEdViewportToolbar::MakeFollowBoneWidget() const
{
	return MakeFollowBoneWidget(nullptr);
}

TSharedRef<SWidget> SActionSkillEdViewportToolbar::MakeFollowBoneWidget(TWeakPtr<class SComboButton> InWeakComboButton) const
{
	TSharedPtr<SBoneTreeMenu> BoneTreeMenu;

	TSharedRef<SWidget> MenuWidget =
		SNew(SBox)
		.MaxDesiredHeight(400.0f)
		[
			SAssignNew(BoneTreeMenu, SBoneTreeMenu)
			.Title(LOCTEXT("FollowBoneTitle", "Skill Camera Follow Bone"))
			.bShowVirtualBones(true)
			.OnBoneSelectionChanged_Lambda([this](FName InBoneName)
			{
				Viewport.Pin()->SetCameraFollowMode(ESkillViewportCameraFollowMode::Bone, InBoneName);
				FSlateApplication::Get().DismissAllMenus();

				PinnedCommands->AddCustomWidget(TEXT("FollowBoneWidget"));
			})
			.SelectedBone(Viewport.Pin()->GetCameraFollowBoneName())
			.OnGetReferenceSkeleton_Lambda([this]() -> const FReferenceSkeleton&
			{
				ACharacter* PreviewCharacter = Viewport.Pin()->GetPreviewScene().Pin()->GetPrevCharacter();
				if (PreviewCharacter)
				{
					return PreviewCharacter->GetMesh()->SkeletalMesh->RefSkeleton;
				}

				static FReferenceSkeleton EmptySkeleton;
				return EmptySkeleton;
			})
		];

	if(InWeakComboButton.IsValid())
	{
		InWeakComboButton.Pin()->SetMenuContentWidgetToFocus(BoneTreeMenu->GetFilterTextWidget());
	}

	return MenuWidget;
}

TSharedRef<SWidget> SActionSkillEdViewportToolbar::MakeFOVWidget() const
{
	const float FOVMin = 5.f;
	const float FOVMax = 170.f;

	return
		SNew(SBox)
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.WidthOverride(100.0f)
			[
				SNew(SNumericEntryBox<float>)
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.AllowSpin(true)
				.MinValue(FOVMin)
				.MaxValue(FOVMax)
				.MinSliderValue(FOVMin)
				.MaxSliderValue(FOVMax)
				.Value_Lambda([this]() -> float {
					return Viewport.Pin()->GetViewportClient()->ViewFOV;
				})
				.OnValueChanged_Lambda([this](float NewValue) {
					Viewport.Pin()->GetViewportClient()->FOVAngle = NewValue;
					Viewport.Pin()->GetViewportClient()->ViewFOV = NewValue;
					Viewport.Pin()->GetViewportClient()->Invalidate();
				})
			]
		];
}
#undef LOCTEXT_NAMESPACE