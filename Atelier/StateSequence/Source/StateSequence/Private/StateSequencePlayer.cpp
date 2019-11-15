#include "StateSequencePlayer.h"
#include "StateSequence.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"


UStateSequencePlayer::UStateSequencePlayer(const FObjectInitializer& Init) : Super(Init)
{
	DesirePlayToFrame = FFrameTime(-1);
	DesireState = -1;
}

UObject* UStateSequencePlayer::GetPlaybackContext() const
{
	UStateSequence* StateSequence = CastChecked<UStateSequence>(Sequence);
	if (StateSequence)
	{
		if (AActor* Actor = StateSequence->GetOutterActor())
		{
			return Actor;
		}
#if WITH_EDITOR
		else if (UBlueprintGeneratedClass* GeneratedClass = StateSequence->GetTypedOuter<UBlueprintGeneratedClass>())
		{
			return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
		}
#endif
	}

	return nullptr;
}

TArray<UObject*> UStateSequencePlayer::GetEventContexts() const
{
	TArray<UObject*> Contexts;
	if (UObject* PlaybackContext = GetPlaybackContext())
	{
		Contexts.Add(PlaybackContext);
	}
	return Contexts;
}

void UStateSequencePlayer::Update(const float DeltaSeconds)
{
	Super::Update(DeltaSeconds);

	if (DesirePlayToFrame >= 0)
	{
		if (IsPlaying())
		{
			FQualifiedFrameTime CurrentTime = GetCurrentTime();
			if (CurrentTime.Time.FrameNumber == DesirePlayToFrame.FrameNumber)
			{
				Pause();
			}
		}
		else
		{
			UStateSequence* StateSequence = CastChecked<UStateSequence>(Sequence);
			StateSequence->Notify(DesireState);
			DesirePlayToFrame = FFrameTime(-1);
			if (ReachedTheStateCallback.IsBound())
			{
				ReachedTheStateCallback.Execute();
				ReachedTheStateCallback.Unbind();
			}
		}
	}
}

FFrameTime UStateSequencePlayer::ClampTime(FFrameTime InTime, FFrameTime MinTime, FFrameTime MaxTime)
{
	if (InTime < MinTime) return MinTime;
	if (InTime > MaxTime) return MaxTime;

	return InTime;
}

void UStateSequencePlayer::PlayToFrame(FFrameTime NewPosition)
{
	DesirePlayToFrame = ClampTime(NewPosition, StartTime, StartTime + GetFrameDuration());
	TimeController->Reset(GetCurrentTime());
}

void UStateSequencePlayer::JumpToFrame(FFrameTime NewPosition)
{
	DesirePlayToFrame = FFrameTime(-1);

	UpdateTimeCursorPosition(NewPosition, EUpdatePositionMethod::Jump);

	TimeController->Reset(GetCurrentTime());
}

void UStateSequencePlayer::PlayToSeconds(float TimeInSeconds, int32 StateIndex, FReachedTheStateCallback callback)
{
	bool playback = DesireState > StateIndex;
	DesireState = StateIndex;
	ReachedTheStateCallback = callback;
	PlayToFrame(TimeInSeconds * PlayPosition.GetInputRate());
	if (playback) 
	{
		PlayReverse();
	}
	else
	{
		Play();
	}
}

void UStateSequencePlayer::JumpToSeconds(float TimeInSeconds, int32 StateIndex)
{
	DesireState = StateIndex;

	JumpToFrame(TimeInSeconds * PlayPosition.GetInputRate());

	UStateSequence* StateSequence = CastChecked<UStateSequence>(Sequence);
	StateSequence->Notify(DesireState);
}
