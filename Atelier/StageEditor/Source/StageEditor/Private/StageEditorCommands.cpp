// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "StageEditorCommands.h"

#define LOCTEXT_NAMESPACE "FStageEditorModule"

void FStageEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "StageEditor", "Bring up StageEditor window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
