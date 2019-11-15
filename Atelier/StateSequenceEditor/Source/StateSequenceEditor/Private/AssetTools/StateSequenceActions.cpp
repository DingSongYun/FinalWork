// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31
#include "StateSequenceActions.h"
#include "StateSequence.h"
#include "AssetTypeCategories.h"
#include "SDockTab.h"
#include "LevelEditor.h"
#include "StateSequenceEditorTabSummoner.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FStateSequenceActions::FStateSequenceActions(const TSharedRef<ISlateStyle>& InStyle)
    : Style(InStyle)
{}

uint32 FStateSequenceActions::GetCategories()
{
    return EAssetTypeCategories::Animation;
}

FText FStateSequenceActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_StateSequencee", "State Sequence");

}

UClass* FStateSequenceActions::GetSupportedClass() const
{
    return UStateSequence::StaticClass(); 
}

FColor FStateSequenceActions::GetTypeColor() const
{
    return FColor(200, 80, 80);
}

FName SequenceTabId("StateSequenceID");
void FStateSequenceActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	UWorld* WorldContext = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			WorldContext = Context.World();
			break;
		}
	}

	if (!ensure(WorldContext))
	{
		return;
	}

	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UStateSequence* StateSequence = Cast<UStateSequence>(*ObjIt);

		if (LevelEditorTabManager->CanSpawnTab(SequenceTabId))
		{
			LevelEditorTabManager->InvokeTab(SequenceTabId)->RequestCloseTab();
			TSharedRef<SDockTab> Tab = LevelEditorTabManager->InvokeTab(SequenceTabId);
			StaticCastSharedRef<SStateSequenceEditorWidget>(Tab->GetContent())->Initialize(Mode, EditWithinLevelEditor,StateSequence);
		}
	}
}

bool FStateSequenceActions::ShouldForceWorldCentric()
{
	return true;
}

#undef LOCTEXT_NAMESPACE