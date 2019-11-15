// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-30

#include "StateSequence.h"
#include "MovieScene.h"
#include "Animation/AnimInstance.h"
#include "Tracks/MovieSceneStateTrack.h"
#include "Sections/MovieSceneStateSection.h"
#include "Core/Public/Modules/ModuleManager.h"

#if WITH_EDITORONLY_DATA
#include "ClassViewerModule.h"
#endif

#define LOCTEXT_NAMESPACE "StateSequence"

UStateSequence::UStateSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// do nothing here
}

void UStateSequence::Initialize()
{
	MovieScene = NewObject<UMovieScene>(this, NAME_None, RF_Transactional);
	const bool bFrameLocked = false;

	MovieScene->SetEvaluationType( bFrameLocked ? EMovieSceneEvaluationType::FrameLocked : EMovieSceneEvaluationType::WithSubFrames );

	FFrameRate TickResolution(60000, 1);
	//TryParseString(TickResolution, *CVarDefaultTickResolution.GetValueOnGameThread());
	MovieScene->SetTickResolutionDirectly(TickResolution);

	FFrameRate DisplayRate(30, 1);
	MovieScene->SetDisplayRate(DisplayRate);

	// Add default state track
	//MovieScene->FindMasterTrack<UMovieSceneStateTrack>();
	UMovieSceneStateTrack* NewTrack = MovieScene->AddMasterTrack<UMovieSceneStateTrack>();
	check(NewTrack);

	UMovieSceneSection* NewSection = NewTrack->CreateNewSection();
	check(NewSection);
	
	NewTrack->AddSection(*NewSection);

#if WITH_EDITOR
	NewTrack->SetDisplayName(LOCTEXT("TrackName", "State"));
#endif
}

void UStateSequence::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	if (Context)
	{
		if (AActor* BindingActor = Cast<AActor>(&PossessedObject))
		{
			BindingReferences.AddBinding(ObjectId, BindingActor, BindingActor);
		}
		else
		{
			BindingReferences.AddBinding(ObjectId, &PossessedObject, Context);
		}
	}
}

bool UStateSequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	return Object.IsA<AActor>() || Object.IsA<UActorComponent>() || Object.IsA<UAnimInstance>();
}

void UStateSequence::LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	if (Context)
	{
		BindingReferences.ResolveBinding(ObjectId, Context, OutObjects);
	}
}

UMovieScene* UStateSequence::GetMovieScene() const
{
	return MovieScene;
}

UObject* UStateSequence::GetParentObject(UObject* Object) const
{
	if (UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		return Component->GetOwner();
	}


	if (UAnimInstance* AnimInstance = Cast<UAnimInstance>(Object))
	{
		if (AnimInstance->GetWorld())
		{
			return AnimInstance->GetOwningComponent();
		}
	}

	return nullptr;	
}

void UStateSequence::UnbindPossessableObjects(const FGuid& ObjectId)
{
	BindingReferences.RemoveBinding(ObjectId);
}

FGuid UStateSequence::GetRootBinding()
{
	return BindingReferences.GetRootBinding();
}

FGuid UStateSequence::AssignActor(AActor* InActor)
{
	if (InActor)
	{
		OutterActor = InActor;
		FGuid RootBinding = GetRootBinding();
		if (RootBinding.IsValid())
		{
			BindPossessableObject(RootBinding, *InActor, InActor);
			return RootBinding;
		}
	}

	return FGuid();
}

const class UMovieSceneStateTrack* UStateSequence::GetStateTrack() const
{
	return MovieScene->FindMasterTrack<UMovieSceneStateTrack>();
}

const FMovieSceneStateSectionData& UStateSequence::GetStateData() const
{
	if (const UMovieSceneStateTrack* StateTrack = GetStateTrack())
	{
		auto* StateSection = Cast<UMovieSceneStateSection>(StateTrack->GetAllSections()[0]);

		return StateSection->GetStateData();
	}

	return *(new FMovieSceneStateSectionData());
}

bool UStateSequence::CanAnimateObject(UObject& InObject) const
{
	return InObject.IsA<AActor>() || InObject.IsA<UActorComponent>() || InObject.IsA<UAnimInstance>();
}

void UStateSequence::GatherExpiredObjects(const FMovieSceneObjectCache& InObjectCache, TArray<FGuid>& OutInvalidIDs) const
{
	//for (const FGuid& ObjectId : BindingReferences.GetBoundAnimInstances())
	//{
	//	for (TWeakObjectPtr<> WeakObject : InObjectCache.IterateBoundObjects(ObjectId))
	//	{
	//		UAnimInstance* AnimInstance = Cast<UAnimInstance>(WeakObject.Get());
	//		if (!AnimInstance || !AnimInstance->GetOwningComponent() || AnimInstance->GetOwningComponent()->GetAnimInstance() != AnimInstance)
	//		{
	//			OutInvalidIDs.Add(ObjectId);
	//		}
	//	}
	//}
}

void UStateSequence::Notify(int32 index)
{
	GetStateData().NotifyAll(GetOutterActor(), index);
}

#undef LOCTEXT_NAMESPACE 

IMPLEMENT_MODULE(FDefaultModuleImpl, StateSequence);