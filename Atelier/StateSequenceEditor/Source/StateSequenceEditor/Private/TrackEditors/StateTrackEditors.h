// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05

#pragma once

#include "MovieSceneTrackEditor.h"
#include "Reply.h"

class FStateTrackEditor : public FMovieSceneTrackEditor
{
public:
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

public:
	FStateTrackEditor(TSharedRef<ISequencer> InSequencer);

public:

	//~ Begin: ISequencerTrackEditor interface
	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass) override;
	virtual bool SupportsType( TSubclassOf<UMovieSceneTrack> Type ) const override;
	virtual bool SupportsSequence(UMovieSceneSequence* InSequence) const override;
	virtual const FSlateBrush* GetIconBrush() const override;
	virtual void BuildTrackContextMenu( FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track ) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;

	//~ End: ISequencerTrackEditor interface

private:

	/** Callback for executing the "Add Event Track" menu entry. */
	void HandleAddEventTrackMenuEntryExecute();
};