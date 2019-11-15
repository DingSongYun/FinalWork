// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31

#include "StateSequenceFactory.h"
#include "StateSequence.h"
#include "MovieScene.h"
#include "MovieSceneToolsProjectSettings.h"

#define LOCTEXT_NAMESPACE "MovieSceneFactory"


/* ULevelSequenceFactory structors
 *****************************************************************************/

UStateSequenceFactory::UStateSequenceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UStateSequence::StaticClass();
}


/* UFactory interface
 *****************************************************************************/

UObject* UStateSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewStateSequence = NewObject<UStateSequence>(InParent, Name, Flags|RF_Transactional);
	NewStateSequence->Initialize();

	// Set up some sensible defaults
	const UMovieSceneToolsProjectSettings* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();

	FFrameRate TickResolution = NewStateSequence->GetMovieScene()->GetTickResolution();
	NewStateSequence->GetMovieScene()->SetPlaybackRange((ProjectSettings->DefaultStartTime*TickResolution).FloorToFrame(), (ProjectSettings->DefaultDuration*TickResolution).FloorToFrame().Value);

	return NewStateSequence;
}


bool UStateSequenceFactory::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE