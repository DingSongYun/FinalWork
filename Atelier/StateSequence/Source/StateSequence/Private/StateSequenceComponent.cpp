// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-30

#include "StateSequenceComponent.h"
#include "StateSequence.h"
#include "StateSequencePlayer.h"
#include "Sections/MovieSceneStateSection.h"

UStateSequenceComponent::UStateSequenceComponent(const FObjectInitializer& Init)
{
	PrimaryComponentTick.bCanEverTick = true;
	PlaybackSettings.bPauseAtEnd = true;
	State = -1;
}

void UStateSequenceComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void UStateSequenceComponent::BeginPlay()
{
	Super::BeginPlay();

	SequencePlayer = NewObject<UStateSequencePlayer>(this, "SequencePlayer");
	if (Sequence)
	{
		Initialize(Sequence);
		TransiteStateTo(State, FReachedTheStateCallback());
	}
}

void UStateSequenceComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SequencePlayer)
	{
		SequencePlayer->Update(DeltaTime);
	}
}

void UStateSequenceComponent::Initialize(class UStateSequence* InSequence)
{
	if (InSequence != nullptr)
	{
		Sequence = InSequence;
		Sequence->AssignActor(GetOwner());
		SequencePlayer->Initialize(Sequence, PlaybackSettings);
		TransiteStateTo(0, FReachedTheStateCallback());
	}
}

void UStateSequenceComponent::TransiteStateTo(int32 StateIndex, FReachedTheStateCallback Callback, bool bInstant)
{
	if ( !IsSequencerValid() || !IsValidStateIndex(StateIndex)) return;
	
	if (StateIndex == State) return;

	auto StateData = Sequence->GetStateData();
	TArrayView<const FFrameNumber> KeyTimes = StateData.GetKeyTimes();
	//TArrayView<const FStatePayload> KeyValues = StateData.GetKeyValues();
	double KeyTime = ((FFrameTime)KeyTimes[StateIndex]) / Sequence->GetMovieScene()->GetTickResolution();

	if (bInstant)
	{
		SequencePlayer->JumpToSeconds(KeyTime, StateIndex);
		if (Callback.IsBound())
		{
			Callback.Execute();
		}
	}
	else
	{
		SequencePlayer->PlayToSeconds(KeyTime, StateIndex, Callback);
	}

	State = StateIndex;
}

int32 UStateSequenceComponent::GetStateNum()
{
	if (IsSequencerValid())
	{
		return Sequence->GetStateData().GetNumKeys();
	}
	return 0;
}

bool UStateSequenceComponent::IsSequencerValid()
{
	return Sequence != nullptr;
}

bool UStateSequenceComponent::IsValidStateIndex(int32 StateIndex)
{
	return IsSequencerValid() && StateIndex >= 0 && StateIndex < GetStateNum();
}
