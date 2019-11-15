// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "StageEditorStyle.h"

class FStageEditorCommands : public TCommands<FStageEditorCommands>
{
public:

	FStageEditorCommands()
		: TCommands<FStageEditorCommands>(TEXT("StageEditor"), NSLOCTEXT("Contexts", "StageEditor", "StageEditor Plugin"), NAME_None, FStageEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};