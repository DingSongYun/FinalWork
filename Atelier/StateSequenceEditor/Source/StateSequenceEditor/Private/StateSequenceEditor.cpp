// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31

#include "StateSequenceEditor.h"
#include "AssetToolsModule.h"
#include "AssetTools/StateSequenceActions.h"
#include "StateSequenceEditorStyle.h"
#include "StateSequenceEditorTabSummoner.h"
#include "SDockTab.h"
#include "LevelEditor.h"
#include "ISequencerModule.h"
#include "TrackEditors/StateTrackEditors.h"
#include "TrackEditors/MaterialParameterStateTrackEditor.h"
#include "MovieScene.h"
#include "StateClipboardTypes.h"
#include "ISequencerChannelInterface.h"
#include "SequencerChannelInterface.h"
#include "Sections/MovieSceneStateSection.h"
#include "SequencerChannelTraits.h"
#include "MultiBoxBuilder.h"
#include "Text.h"

#define LOCTEXT_NAMESPACE "FStateSequenceEditorModule"

TSharedPtr<FStateSequenceEditorStyle> FStateSequenceEditorStyle::Singleton;

class FStateSequenceEditorTabBinding
	: public TSharedFromThis<FStateSequenceEditorTabBinding>
{
public:
	FStateSequenceEditorTabBinding()
	{
		if (GIsEditor)
		{
			FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorTabSpawnerHandle = LevelEditor.OnRegisterTabs().AddRaw(this, &FStateSequenceEditorTabBinding::RegisterLevelEditorTab);
		}
	}

	~FStateSequenceEditorTabBinding()
	{
		if (GIsEditor)
		{
			FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
			if (LevelEditor)
			{
				LevelEditor->OnRegisterTabs().Remove(LevelEditorTabSpawnerHandle);
			}
		}
	}

private:
	void RegisterLevelEditorTab(TSharedPtr<FTabManager> InTabManager)
	{
		auto SpawnTab = [](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
			{
				TSharedRef<SStateSequenceEditorWidget> Widget = SNew(SStateSequenceEditorWidget, nullptr);

				return SNew(SDockTab)
					.Label(&Widget.Get(), &SStateSequenceEditorWidget::GetDisplayLabel)
					.Icon(FStateSequenceEditorStyle::Get()->GetBrush("ClassIcon.ActorSequence"))
					[
						Widget
					];
			};

		InTabManager->RegisterTabSpawner("StateSequenceID", FOnSpawnTab::CreateStatic(SpawnTab))
			.SetMenuType(ETabSpawnerMenuType::Hidden)
			.SetAutoGenerateMenuEntry(false);
	}
private:
	FDelegateHandle LevelEditorTabSpawnerHandle;
};

void FStateSequenceEditorModule::StartupModule()
{
	// Register styles
	FStateSequenceEditorStyle::Get();

	// Register Tab
	BlueprintEditorTabBinding = MakeShared<FStateSequenceEditorTabBinding>();

	// Register Asset Tools
	RegisterAssetTools();

	// Register Sequence Track Editor
	RegisterSequenceTrackEditor();
}

void FStateSequenceEditorModule::ShutdownModule()
{
	BlueprintEditorTabBinding = nullptr;

	// unregister Asset Tools
	UnregisterAssetTools();

	// Unregister Sequence Track Editor
	UnregisterSequenceTrackEditor();
}

void FStateSequenceEditorModule::RegisterAssetTools()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FStateSequenceActions(FStateSequenceEditorStyle::Get())));
}

void FStateSequenceEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

void FStateSequenceEditorModule::UnregisterAssetTools()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

	if (AssetToolsModule != nullptr)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

void FStateSequenceEditorModule::RegisterSequenceTrackEditor()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>( "Sequencer" );
	SequenceTrackCreateEditorHandles.Add(SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FStateTrackEditor::CreateTrackEditor)));
	SequenceTrackCreateEditorHandles.Add(SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMaterialParameterStateTrackEditor::CreateTrackEditor)));
	SequencerModule.RegisterChannelInterface<FMovieSceneStateSectionData>();
	// SequencerModule.RegisterChannelInterface(TUniquePtr<ISequencerChannelInterface>(new TStateSequencerChannelInterface()));
}

void FStateSequenceEditorModule::UnregisterSequenceTrackEditor()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>( "Sequencer" );
	for (FDelegateHandle& Handle : SequenceTrackCreateEditorHandles)
	{
		SequencerModule.UnRegisterTrackEditor( Handle );
	}
	SequenceTrackCreateEditorHandles.Empty();
}

namespace Sequencer 
{
	template<>
	void ExtendKeyMenu(FMenuBuilder& MenuBuilder, TArray<TExtendKeyMenuParams<FMovieSceneStateSectionData>>&& InChannels, TWeakPtr<ISequencer> InSequencer)
	{
		FMovieSceneStateSectionData* data = InChannels[0].Channel.Get();
		UMovieScene* MovieScene = InSequencer.Pin()->GetFocusedMovieSceneSequence()->GetMovieScene();
		data->InSequencer = InSequencer;
		data->DrawNotifyInfo(MenuBuilder, InChannels[0].Handles[0], MovieScene);
	}
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStateSequenceEditorModule, StateSequenceEditor)