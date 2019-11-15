// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05

#include "Tracks/MovieSceneStateTrack.h"
#include "Sections/MovieSceneStateSection.h"
#include "Compilation/IMovieSceneTemplateGenerator.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"

void UMovieSceneStateTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}

UMovieSceneSection* UMovieSceneStateTrack::CreateNewSection()
{
	return NewObject<UMovieSceneSection>(this, UMovieSceneStateSection::StaticClass(), NAME_None, RF_Transactional);
}

const TArray<UMovieSceneSection*>& UMovieSceneStateTrack::GetAllSections() const
{
	return Sections;
}

bool UMovieSceneStateTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}

bool UMovieSceneStateTrack::IsEmpty() const
{
	return Sections.Num() == 0;
}

void UMovieSceneStateTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}

void UMovieSceneStateTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

//FMovieSceneEvalTemplatePtr UMovieSceneStateTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
//{
//	return FMovieSceneStateSectionTemplate(*CastChecked<UMovieSceneStateSection>(&InSection), *this);
//}

void UMovieSceneStateTrack::PostCompile(FMovieSceneEvaluationTrack& Track, const FMovieSceneTrackCompilerArgs& Args) const
{
	Track.SetEvaluationMethod(EEvaluationMethod::Swept);
}