// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-30

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "MovieSceneSequencePlayer.h"
#include "StateSequencePlayer.h"
#include "StateSequenceComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=Sequence, meta=(BlueprintSpawnableComponent))
class STATESEQUENCE_API UStateSequenceComponent : public UActorComponent
{
public:
	GENERATED_BODY()

	UStateSequenceComponent(const FObjectInitializer& Init);

	FORCEINLINE class UStateSequence* GetSequence() const
	{
		return Sequence;
	}

	FORCEINLINE class UStateSequencePlayer* GetSequencePlayer() const
	{
		return SequencePlayer;
	}

	UFUNCTION(BlueprintCallable) 
	void Initialize(class UStateSequence* Sequence);

	//~ Begin: UActorComponent Interface
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//~ End: UActorComponent Interface

	UFUNCTION(BlueprintCallable)
	void TransiteStateTo(int32 StateIndex, FReachedTheStateCallback Callback, bool bInstant = true);

public:
	int32 GetStateNum();
	bool IsValidStateIndex(int32 StateIndex);

	bool IsSequencerValid();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 State;

	UPROPERTY(EditAnywhere, Category="StateSequence")
	class UStateSequence* Sequence;

	UPROPERTY(transient, BlueprintReadOnly, Category="StateSequence")
	class UStateSequencePlayer* SequencePlayer;

	UPROPERTY(EditAnywhere, Category="StateSequence", meta=(ShowOnlyInnerProperties, AdvancedDisplay))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;
};
