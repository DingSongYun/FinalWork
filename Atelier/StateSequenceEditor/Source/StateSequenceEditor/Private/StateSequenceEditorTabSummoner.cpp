// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31

#include "StateSequenceEditorTabSummoner.h"

#include "StateSequence.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "LevelEditorSequencerIntegration.h"
#include "SSCSEditor.h"
#include "Styling/SlateIconFinder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "MovieSceneCaptureDialogModule.h"
#include "StateSequenceEditorStyle.h"


#define LOCTEXT_NAMESPACE "StateSequenceEditorSummoner"

DECLARE_DELEGATE_OneParam(FOnComponentSelected, TSharedPtr<FSCSEditorTreeNode>);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIsComponentValid, UActorComponent*);


class SComponentSelectionTree
	: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SComponentSelectionTree) : _IsInEditMode(false) {}

		SLATE_EVENT(FOnComponentSelected, OnComponentSelected)
		SLATE_EVENT(FIsComponentValid, IsComponentValid)
		SLATE_ARGUMENT(bool, IsInEditMode)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, AActor* InPreviewActor)
	{
		bIsInEditMode = InArgs._IsInEditMode;
		OnComponentSelected = InArgs._OnComponentSelected;
		IsComponentValid = InArgs._IsComponentValid;

		ChildSlot
		[
			SAssignNew(TreeView, STreeView<TSharedPtr<FSCSEditorTreeNode>>)
			.TreeItemsSource(&RootNodes)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SComponentSelectionTree::GenerateRow)
			.OnGetChildren(this, &SComponentSelectionTree::OnGetChildNodes)
			.OnSelectionChanged(this, &SComponentSelectionTree::OnSelectionChanged)
			.ItemHeight(24)
		];

		BuildTree(InPreviewActor);

		if (RootNodes.Num() == 0)
		{
			ChildSlot
			[
				SNew(SBox)
				.Padding(FMargin(5.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoValidComponentsFound", "No valid components available"))
				]
			];
		}
	}

	void BuildTree(AActor* Actor)
	{
		RootNodes.Reset();
		ObjectToNode.Reset();

		for (UActorComponent* Component : TInlineComponentArray<UActorComponent*>(Actor))
		{
			if (IsComponentVisibleInTree(Component))
			{
				FindOrAddNodeForComponent(Component);
			}
		}
	}

private:

	void OnSelectionChanged(TSharedPtr<FSCSEditorTreeNode> InNode, ESelectInfo::Type SelectInfo)
	{
		OnComponentSelected.ExecuteIfBound(InNode);
	}

	void OnGetChildNodes(TSharedPtr<FSCSEditorTreeNode> InNodePtr, TArray<TSharedPtr<FSCSEditorTreeNode>>& OutChildren)
	{
		OutChildren = InNodePtr->GetChildren();
	}

	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FSCSEditorTreeNode> InNodePtr, const TSharedRef<STableViewBase>& OwnerTable)
	{
		const FSlateBrush* ComponentIcon = FEditorStyle::GetBrush("SCS.NativeComponent");
		if (InNodePtr->GetComponentTemplate() != NULL)
		{
			ComponentIcon = FSlateIconFinder::FindIconBrushForClass( InNodePtr->GetComponentTemplate()->GetClass(), TEXT("SCS.Component") );
		}

		FText Label = InNodePtr->IsInherited() && !bIsInEditMode
			? FText::Format(LOCTEXT("NativeComponentFormatString","{0} (Inherited)"), FText::FromString(InNodePtr->GetDisplayString()))
			: FText::FromString(InNodePtr->GetDisplayString());

		TSharedRef<STableRow<FSCSEditorTreeNodePtrType>> Row = SNew(STableRow<FSCSEditorTreeNodePtrType>, OwnerTable).Padding(FMargin(0.f, 0.f, 0.f, 4.f));
		Row->SetContent(
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(ComponentIcon)
				.ColorAndOpacity(SSCS_RowWidget::GetColorTintForIcon(InNodePtr))
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(Label)
			]);

		return Row;
	}

	bool IsComponentVisibleInTree(UActorComponent* ActorComponent) const
	{
		return !IsComponentValid.IsBound() || IsComponentValid.Execute(ActorComponent);
	}

	TSharedPtr<FSCSEditorTreeNode> FindOrAddNodeForComponent(UActorComponent* ActorComponent)
	{
		if (ActorComponent->IsEditorOnly())
		{
			return nullptr;
		}

		if (TSharedPtr<FSCSEditorTreeNode>* Existing = ObjectToNode.Find(ActorComponent))
		{
			return *Existing;
		}
		else if (USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
		{
			if (UActorComponent* Parent = SceneComponent->GetAttachParent())
			{
				TSharedPtr<FSCSEditorTreeNode> ParentNode = FindOrAddNodeForComponent(Parent);

				if (!ParentNode.IsValid())
				{
					return nullptr;
				}

				TreeView->SetItemExpansion(ParentNode, true);

				TSharedPtr<FSCSEditorTreeNode> ChildNode = ParentNode->AddChildFromComponent(ActorComponent);
				ObjectToNode.Add(ActorComponent, ChildNode);

				return ChildNode;
			}
		}
		
		TSharedPtr<FSCSEditorTreeNode> RootNode = FSCSEditorTreeNode::FactoryNodeFromComponent(ActorComponent);
		RootNodes.Add(RootNode);
		ObjectToNode.Add(ActorComponent, RootNode);

		TreeView->SetItemExpansion(RootNode, true);

		return RootNode;
	}

private:
	bool bIsInEditMode;
	FOnComponentSelected OnComponentSelected;
	FIsComponentValid IsComponentValid;
	TSharedPtr<STreeView<TSharedPtr<FSCSEditorTreeNode>>> TreeView;
	TMap<FObjectKey, TSharedPtr<FSCSEditorTreeNode>> ObjectToNode;
	TArray<TSharedPtr<FSCSEditorTreeNode>> RootNodes;
};

class SStateSequenceEditorWidgetImpl : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SStateSequenceEditorWidgetImpl){}
	SLATE_END_ARGS();

	~SStateSequenceEditorWidgetImpl()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
			Sequencer->Close();
			Sequencer = nullptr;
		}

		GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileHandle);
		FCoreUObjectDelegates::OnObjectSaved.Remove(OnObjectSavedHandle);
	}

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
	{
		OnBlueprintPreCompileHandle = GEditor->OnBlueprintPreCompile().AddSP(this, &SStateSequenceEditorWidgetImpl::OnBlueprintPreCompile);
		OnObjectSavedHandle = FCoreUObjectDelegates::OnObjectSaved.AddSP(this, &SStateSequenceEditorWidgetImpl::OnObjectPreSave);

		WeakBlueprintEditor = InBlueprintEditor;

		ChildSlot
		[
			SAssignNew(Content, SBox)
			.MinDesiredHeight(200)
		];
	}

	FText GetDisplayLabel() const
	{
		UStateSequence* Sequence = WeakSequence.Get();
		return Sequence ? Sequence->GetDisplayName() : LOCTEXT("DefaultSequencerLabel", "Sequencer");
	}

	UStateSequence* GetStateSequence() const
	{
		return WeakSequence.Get();
	}

	// same as LevelSequenceEditorTookKit
	UObject* GetPlaybackContext() const
	{
		if (PlaybackContextObject) return PlaybackContextObject;

		UWorld* PIEWorld = nullptr;
		UWorld* EditorWorld = nullptr;

		IMovieSceneCaptureDialogModule* CaptureDialogModule = FModuleManager::GetModulePtr<IMovieSceneCaptureDialogModule>("MovieSceneCaptureDialog");
		UWorld* RecordingWorld = CaptureDialogModule ? CaptureDialogModule->GetCurrentlyRecordingWorld() : nullptr;

		bool bIsSimulatingInEditor = GEditor && GEditor->bIsSimulatingInEditor;
		/*bool bUsePIEWorld = (!bIsSimulatingInEditor && GetDefault<ULevelEditorPlaySettings>()->bBindSequencerToPIE)
						|| (bIsSimulatingInEditor && GetDefault<ULevelEditorPlaySettings>()->bBindSequencerToSimulate);*/
		// jianan ,临时设置成true，保证能编译过。
		bool bUsePIEWorld = true;

		// Return PIE worlds if there are any
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				UWorld* ThisWorld = Context.World();
				if (bUsePIEWorld && RecordingWorld != ThisWorld)
				{
					PIEWorld = ThisWorld;
				}
			}
			else if (Context.WorldType == EWorldType::Editor)
			{
				// We can always animate PIE worlds
				EditorWorld = Context.World();
				if (!bUsePIEWorld)
				{
					return EditorWorld;
				}
			}
		}

		return PIEWorld ? PIEWorld : EditorWorld;
	}

	TArray<UObject*> GetEventContexts() const
	{
		TArray<UObject*> Contexts;
		if (auto* Context = GetPlaybackContext())
		{
			Contexts.Add(Context);
		}
		return Contexts;
	}

	void Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UStateSequence* NewSequence)
	{
		if (UStateSequence* OldSequence = WeakSequence.Get())
		{
			if (OnSequenceChangedHandle.IsValid())
			{
				OldSequence->OnSignatureChanged().Remove(OnSequenceChangedHandle);
			}
		}

		WeakSequence = NewSequence;

		if (NewSequence)
		{
			OnSequenceChangedHandle = NewSequence->OnSignatureChanged().AddSP(this, &SStateSequenceEditorWidgetImpl::OnSequenceChanged);
		}

		// If we already have a sequencer open, just assign the sequence
		if (Sequencer.IsValid() && NewSequence)
		{
			if (Sequencer->GetRootMovieSceneSequence() != NewSequence)
			{
				Sequencer->ResetToNewRootSequence(*NewSequence);
			}
			return;
		}

		// If we're setting the sequence to none, destroy sequencer
		if (!NewSequence)
		{
			if (Sequencer.IsValid())
			{
				FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
				Sequencer->Close();
				Sequencer = nullptr;
			}

			Content->SetContent(SNew(STextBlock).Text(LOCTEXT("NothingSelected", "Select a sequence")));
			return;
		}

		// We need to initialize a new sequencer instance
		FSequencerInitParams SequencerInitParams;
		{
			TWeakObjectPtr<UStateSequence> LocalWeakSequence = NewSequence;

			SequencerInitParams.RootSequence = NewSequence;
			SequencerInitParams.EventContexts = TAttribute<TArray<UObject*>>(this, &SStateSequenceEditorWidgetImpl::GetEventContexts);
			SequencerInitParams.PlaybackContext = TAttribute<UObject*>(this, &SStateSequenceEditorWidgetImpl::GetPlaybackContext);

			TSharedRef<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			TSharedRef<FExtender> AddMenuExtender = MakeShareable(new FExtender);

			ToolbarExtender->AddToolBarExtension("Base Commands", EExtensionHook::Before, nullptr,
				FToolBarExtensionDelegate::CreateLambda([=](FToolBarBuilder& ToolBarBuilder){

					ToolBarBuilder.AddToolBarButton(
						FUIAction(FExecuteAction::CreateSP(this, &SStateSequenceEditorWidgetImpl::PossessSelection)),
						NAME_None,
						LOCTEXT("Possess_CurrSelected_Label", "Possess Selection"),
						LOCTEXT("Possess_CurrSelected_ToolTip", "Possess current selected object to play the sequence"),
						FSlateIcon(FStateSequenceEditorStyle::Get()->GetStyleSetName(), "StateSequencer.Assign")
					);

				})
			);
			AddMenuExtender->AddMenuExtension("AddTracks", EExtensionHook::Before, nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder){

					MenuBuilder.AddSubMenu(
						LOCTEXT("AddComponent_Label", "Component"),
						LOCTEXT("AddComponent_ToolTip", "Add a binding to one of this actor's components and allow it to be animated by Sequencer"),
						FNewMenuDelegate::CreateRaw(this, &SStateSequenceEditorWidgetImpl::AddPossessComponentMenuExtensions),
						false /*bInOpenSubMenuOnClick*/,
						FSlateIcon()//"LevelSequenceEditorStyle", "LevelSequenceEditor.PossessNewActor")
						);

				})
			);


			SequencerInitParams.ViewParams.bReadOnly = false;
			SequencerInitParams.bEditWithinLevelEditor = true;
			SequencerInitParams.ToolkitHost = InitToolkitHost;
			SequencerInitParams.ViewParams.AddMenuExtender = AddMenuExtender;
			SequencerInitParams.ViewParams.ToolbarExtender = ToolbarExtender;
			SequencerInitParams.ViewParams.UniqueName = "EmbeddedStateSequenceEditor";
			SequencerInitParams.ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
			SequencerInitParams.ViewParams.OnReceivedFocus.BindRaw(this, &SStateSequenceEditorWidgetImpl::OnSequencerReceivedFocus);
		}

		Sequencer = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer").CreateSequencer(SequencerInitParams);
		Sequencer->OnActorAddedToSequencer().AddSP(this, &SStateSequenceEditorWidgetImpl::OnActorAddToSequencer);
		Content->SetContent(Sequencer->GetSequencerWidget());

		FLevelEditorSequencerIntegrationOptions Options;
		Options.bRequiresLevelEvents = true;
		Options.bRequiresActorEvents = false;
		Options.bCanRecord = false;

		FLevelEditorSequencerIntegration::Get().AddSequencer(Sequencer.ToSharedRef(), Options);
	}

	void OnSequencerReceivedFocus()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().OnSequencerReceivedFocus(Sequencer.ToSharedRef());
		}
	}

	void OnObjectPreSave(UObject* InObject)
	{
		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (Sequencer.IsValid() && BlueprintEditor.IsValid() && InObject && InObject == BlueprintEditor->GetBlueprintObj())
		{
			Sequencer->RestorePreAnimatedState();
		}
	}

	void OnBlueprintPreCompile(UBlueprint* InBlueprint)
	{
		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (Sequencer.IsValid() && BlueprintEditor.IsValid() && InBlueprint && InBlueprint == BlueprintEditor->GetBlueprintObj())
		{
			Sequencer->RestorePreAnimatedState();
		}
	}

	void OnSelectionUpdated(TSharedPtr<FSCSEditorTreeNode> SelectedNode)
	{
		if (SelectedNode->GetNodeType() != FSCSEditorTreeNode::ComponentNode)
		{
			return;
		}

		UActorComponent* EditingComponent = nullptr;

		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (BlueprintEditor.IsValid())
		{
			UBlueprint* Blueprint = BlueprintEditor->GetBlueprintObj();
			if (Blueprint)
			{
				EditingComponent = SelectedNode->GetEditableComponentTemplate(Blueprint);
			}
		}
		else if (AActor* Actor = GetPreviewActor())
		{
			EditingComponent = SelectedNode->FindComponentInstanceInActor(Actor);
		}

		if (EditingComponent)
		{
			const FScopedTransaction Transaction(LOCTEXT("AddComponentToSequencer", "Add component to Sequencer"));
			Sequencer->GetHandleToObject(EditingComponent, true);
		}

		FSlateApplication::Get().DismissAllMenus();
	}

	void PossessSelection()
	{
		auto IsActorValidForPossession = [=](const AActor* InActor, TWeakPtr<ISequencer> InWeakSequencer)
		{
			TSharedPtr<ISequencer> SequencerPtr = InWeakSequencer.Pin();
			bool bCreateHandleIfMissing = false;
			return SequencerPtr.IsValid() && !SequencerPtr->GetHandleToObject((UObject*)InActor, bCreateHandleIfMissing).IsValid();
		};
		TArray<AActor*> ActorsValidForPossession;
		GEditor->GetSelectedActors()->GetSelectedObjects(ActorsValidForPossession);
		ActorsValidForPossession.RemoveAll([&](AActor* In){ return !IsActorValidForPossession(In, Sequencer); });

		if (ActorsValidForPossession.Num() >= 1)
		{
			PlaybackContextObject = ActorsValidForPossession[0];
			// Sequencer->ForceEvaluate();

			//Sequencer->AddActors();
			//AddActorsToSequencer(&(ActorsValidForPossession[0]), 1);
			AddOrAssignActor(*ActorsValidForPossession[0]);
		}
	}

	void AddOrAssignActor(AActor& InActor)
	{
		FGuid RootBinding = WeakSequence->GetRootBinding();
		if (RootBinding.IsValid())
		{
			AssignActorToSequencer(&InActor, RootBinding);
		}
		else
		{
			AddActorToSequencer(&InActor);
		}
	}


	FGuid AssignActorToSequencer(AActor* InActor, const FGuid& ObjectBinding)
	{
		if (InActor == nullptr) return FGuid();

		UMovieScene* MovieScene = WeakSequence->GetMovieScene();

		InActor->Modify();
		WeakSequence->Modify();
		MovieScene->Modify();

		/**
		 * Simple Assign
		 */
		FGuid Guid = WeakSequence->AssignActor(InActor);
		FormatSpawnable(Guid, InActor);

		Sequencer->RestorePreAnimatedState();
		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);

		return Guid;
	}

	void AddActorToSequencer(AActor* InActor)
	{
		if (InActor == nullptr) return;

		static TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Empty();
		Actors.Add(InActor);


		TArray<FGuid> Guids = Sequencer->AddActors(Actors);
	}

	void OnActorAddToSequencer(AActor* InActor, const FGuid Guid)
	{
		PlaybackContextObject = InActor;
		Sequencer->ForceEvaluate();

		FGuid RootBinding = WeakSequence->GetRootBinding();

		AssignActorToSequencer(InActor, Guid);

		UMovieScene* MovieScene = WeakSequence->GetMovieScene();
		/**
		 * 确保只有一个root track在MovieSequence中
		 * 当有新加的Actor时用新的Actor去替换当前RootTrack的binding
		 * 并删除掉新加的Possessable
		 */
		if (MovieScene->GetPossessableCount() > 1)
		{
			TArray<FMovieSceneBinding> ObjectBindings = MovieScene->GetBindings();
			for(const FMovieSceneBinding& Binding : ObjectBindings)
			{
				if (Binding.GetObjectGuid() != RootBinding)
				{
					MovieScene->RemovePossessable(Binding.GetObjectGuid());
				}
			}
		}
	}

	void FormatSpawnable(const FGuid& ObjectBindingId, const AActor* BindingActor)
	{
		UMovieScene* MovieScene = WeakSequence->GetMovieScene();
		if (FMovieScenePossessable* Possessable = MovieScene->FindPossessable(ObjectBindingId))
		{
			Possessable->SetName("Root(" + BindingActor->GetName() + ")");
		}
	}

	void AddPossessComponentMenuExtensions(FMenuBuilder& MenuBuilder)
	{
		AActor* Actor = GetPreviewActor();
		if (!Actor)
		{
			return;
		}

		Sequencer->State.ClearObjectCaches(*Sequencer);
		TSet<UObject*> AllBoundObjects;

		AllBoundObjects.Add(GetOwnerComponent());

		UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
		for (int32 Index = 0; Index < MovieScene->GetPossessableCount(); ++Index)
		{
			FMovieScenePossessable& Possessable = MovieScene->GetPossessable(Index);
			for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(Possessable.GetGuid(), Sequencer->GetFocusedTemplateID()))
			{
				if (UObject* Object = WeakObject.Get())
				{
					AllBoundObjects.Add(Object);
				}
			}
		}

		bool bIdent = false;
		MenuBuilder.AddWidget(
			SNew(SComponentSelectionTree, Actor)
			.IsInEditMode(true)
			.OnComponentSelected(this, &SStateSequenceEditorWidgetImpl::OnSelectionUpdated)
			.IsComponentValid_Lambda(
				[AllBoundObjects](UActorComponent* Component)
				{
					return !AllBoundObjects.Contains(Component);
				}
			)
			, FText(), !bIdent
		);
	}

	AActor* GetPreviewActor() const
	{
		if (PlaybackContextObject && PlaybackContextObject->IsA<AActor>())
		{
			return Cast<AActor>(PlaybackContextObject);
		}

		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (BlueprintEditor.IsValid())
		{
			return BlueprintEditor->GetPreviewActor();
		}
		if (UStateSequence* Sequence = WeakSequence.Get())
		{
			return Sequence->GetTypedOuter<AActor>();
		}
		return nullptr;
	}

	UActorComponent* GetOwnerComponent() const
	{
		UStateSequence* StateSequence = WeakSequence.Get();
		AActor* Actor = StateSequence ? GetPreviewActor() : nullptr;

		return Actor ? FindObject<UActorComponent>(Actor, *StateSequence->GetOuter()->GetName()) : nullptr;
	}

	void OnSequenceChanged()
	{
		UStateSequence* StateSequence = WeakSequence.Get();
	}

private:
	UObject* PlaybackContextObject;

	TWeakObjectPtr<UStateSequence> WeakSequence;

	TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;

	TSharedPtr<SBox> Content;
	TSharedPtr<ISequencer> Sequencer;

	FDelegateHandle OnBlueprintPreCompileHandle;
	FDelegateHandle OnObjectSavedHandle;

	FDelegateHandle OnSequenceChangedHandle;
};

void SStateSequenceEditorWidget::Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
{
	ChildSlot
	[
		SAssignNew(Impl, SStateSequenceEditorWidgetImpl, InBlueprintEditor)
	];
}

FText SStateSequenceEditorWidget::GetDisplayLabel() const
{
	return Impl.Pin()->GetDisplayLabel();
}

void SStateSequenceEditorWidget::Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UStateSequence* NewStateSequence)
{
	Impl.Pin()->Initialize(Mode, InitToolkitHost, NewStateSequence);
}

UStateSequence* SStateSequenceEditorWidget::GetSequence() const
{
	return Impl.Pin()->GetStateSequence();
}

FStateSequenceEditorSummoner::FStateSequenceEditorSummoner(TSharedPtr<FBlueprintEditor> BlueprintEditor)
	: FWorkflowTabFactory("EmbeddedSequenceID", BlueprintEditor)
	, WeakBlueprintEditor(BlueprintEditor)
{
	bIsSingleton = true;

	TabLabel = LOCTEXT("SequencerTabName", "Sequencer");
}

TSharedRef<SWidget> FStateSequenceEditorSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SStateSequenceEditorWidget, WeakBlueprintEditor);
}

#undef LOCTEXT_NAMESPACE
