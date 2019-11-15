// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-30

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSequencePlayer.h"
#include "StateSequencePlayer.generated.h"

DECLARE_DYNAMIC_DELEGATE(FReachedTheStateCallback);

UCLASS(BlueprintType)
class STATESEQUENCE_API UStateSequencePlayer : public UMovieSceneSequencePlayer
{
	GENERATED_BODY()
public:

	UStateSequencePlayer(const FObjectInitializer&);

	void Update(const float DeltaSeconds);

	void PlayToFrame(FFrameTime NewPosition);

	void JumpToFrame(FFrameTime NewPosition);

	void PlayToSeconds(float TimeInSeconds, int32 StateIndex, FReachedTheStateCallback callback);

	void JumpToSeconds(float TimeInSeconds, int32 StateIndex);

	FFrameTime ClampTime(FFrameTime InTime, FFrameTime MinTime, FFrameTime MaxTime);

protected:
	//~ IMovieScenePlayer interface
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;
	
private:
	FFrameTime DesirePlayToFrame;
	int32 DesireState;
	bool MovieSceneIsPlaying = false;
	FReachedTheStateCallback ReachedTheStateCallback;
};
