// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#include "SWorkshopEditorToolbar.h"
#include "Editor.h"
#include "SImage.h"
#include "SButton.h"
#include "SBoxPanel.h"
#include "SSplitter.h"
#include "SListView.h"
#include "STextBlock.h"
#include "LevelUtils.h"
#include "EditorStyle.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Settings/LevelEditorMiscSettings.h"
#include "ActorFactories/ActorFactory.h"
#include "GameModeInfoCustomizer.h"
#include "DeclarativeSyntaxSupport.h"
#include "Engine/LevelStreaming.h"
#include "EditorUtilities.h"
#include "EngineUtils.h"
#include "AssetDragDropOp.h"
#include "EditorLevelUtils.h"
#include "Model/MapDataModel.h"
#include "JsonObjectConverter.h"
#include "Stage/AreaBox.h"
#include "Stage/StageObject.h"
#include "LevelEditor.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "StageObjectSaveHandler.h"
#include "AssetRegistryModule.h"
#include "AssetSelection.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Utilities/Utilities.h"

DEFINE_LOG_CATEGORY(LogAtelierWorkshopEditor)
const FString WORKSHOP_EDITOR_SCENE_NAME = "WorkshopEditor";


/*****************************************************************/
/************************SWorkshopEditorToolbar************************/
/*****************************************************************/
SWorkshopEditorToolbar::SWorkshopEditorToolbar()
{
}

SWorkshopEditorToolbar::~SWorkshopEditorToolbar()
{
	CleanAll();
}

void SWorkshopEditorToolbar::Construct(const FArguments& InArgs)
{
}

UWorld* SWorkshopEditorToolbar::GetWorld()
{
	return  GEditor->GetEditorWorldContext().World();
}

void SWorkshopEditorToolbar::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
}


void SWorkshopEditorToolbar::CleanAll()
{
	if(EditorTool)
		EditorTool->DestroyEditor();
	EditorTool = nullptr;
}

void SWorkshopEditorToolbar::InitEditorTool()
{
	if (EditorTool)
		return;

	/*
	UWorld * currentWorld = GEditor->LevelViewportClients[0]->GetWorld();
	for (TActorIterator<AWorkshopEditorTool> ActorItr(currentWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		EditorTool = Cast<AWorkshopEditorTool>(*ActorItr);
		if (EditorTool)
		{
			EditorTool->SetupEditor();
			break;
		}
	}
	if (EditorTool != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Find Editor Tool"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find Editor Tool"));
	}
	*/
}