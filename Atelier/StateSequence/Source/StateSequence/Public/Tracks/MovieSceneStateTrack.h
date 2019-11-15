// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05
#pragma once

#include "CoreMinimal.h"
#include "MovieSceneNameableTrack.h"
#include "MovieSceneStateTrack.generated.h"

UCLASS()
class STATESEQUENCE_API UMovieSceneStateTrack
	: public UMovieSceneNameableTrack
{
	GENERATED_BODY()

public:
	UMovieSceneStateTrack()
	{
#if WITH_EDITORONLY_DATA
		TrackTint = FColor(41, 41, 98, 150);
#endif
	}

	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual bool IsEmpty() const override;
	virtual void RemoveAllAnimationData() override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	//virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;
	virtual void PostCompile(FMovieSceneEvaluationTrack& Track, const FMovieSceneTrackCompilerArgs& Args) const override;

private:
	UPROPERTY()
	TArray<class UMovieSceneSection*> Sections;
};