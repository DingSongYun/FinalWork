// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-24

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Editor/UnrealEd/Public/SViewportToolBar.h"
#include "IPinnedCommandList.h"

class SActionSkillEdViewportToolbar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS( SActionSkillEdViewportToolbar ) 
	{}
		SLATE_ARGUMENT(TArray<TSharedPtr<FExtender>>, Extenders)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<class SActionSkillEdViewport> InViewport);

	/** Get the pinned commands widget */
	TSharedPtr<IPinnedCommandList> GetPinnedCommandList() { return PinnedCommands; }

private:
	TSharedRef<SWidget> GenerateViewMenu() const;
	TSharedRef<SWidget> MakeFollowBoneWidget() const;
	TSharedRef<SWidget> MakeFollowBoneWidget(TWeakPtr<class SComboButton> InWeakComboButton) const;
	TSharedRef<SWidget> MakeFOVWidget() const;

private:
	/** The viewport that we are in */
	TWeakPtr<class SActionSkillEdViewport> Viewport;

	/** Pinned commands widget */
	TSharedPtr<IPinnedCommandList> PinnedCommands;

	/** Extenders */
	TArray<TSharedPtr<FExtender>> Extenders;
};