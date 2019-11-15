// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05

#include "StateTrackEditors.h"
#include "EditorStyleSet.h"
#include "MovieSceneTrack.h"
#include "MovieSceneEventTrack.h"
#include "GameFramework/Actor.h"
#include "StateSequence.h"
#include "Tracks/MovieSceneStateTrack.h"
#include "SequencerSectionPainter.h"
#include "Sections/MovieSceneStateSection.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "CommonMovieSceneTools.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FStateTrackEditor"

class FStateSection : public FSequencerSection
{
public:
	FStateSection(UMovieSceneSection& InSectionObject, TWeakPtr<ISequencer> InSequencer)
	: FSequencerSection(InSectionObject)
	, Sequencer(InSequencer)
	{}

public:

	virtual int32 OnPaintSection(FSequencerSectionPainter& Painter) const override
	{
		int32 LayerId = Painter.PaintSectionBackground();

		UMovieSceneStateSection* StateSection = Cast<UMovieSceneStateSection>(WeakSection.Get());
		TSharedPtr<ISequencer> SequencerPtr = Sequencer.Pin();
		TArray<UMovieSceneTrack*> SelectedTracks;
		SequencerPtr->GetSelectedTracks(SelectedTracks);
		
		bool bSelected = false;

		for (auto SelectedTrack : SelectedTracks)
		{
			bSelected = SelectedTrack->GetAllSections().Contains(StateSection);
			if (bSelected) break;
		}

		// Draw state section index and name
		const FTimeToPixel& TimeToPixelConverter = Painter.GetTimeConverter();

		const FMovieSceneStateSectionData& StateData = StateSection->GetStateData();
		for (int32 KeyIndex = 0; KeyIndex < StateData.GetKeyTimes().Num(); ++KeyIndex)
		{
			FFrameNumber KeyTime = StateData.GetKeyTimes()[KeyIndex];
			FStatePayload KeyData = StateData.GetKeyValues()[KeyIndex];

			if (!StateSection->GetRange().Contains(KeyTime))
				continue;

			FString DrawString = bSelected
				? FString::Printf(TEXT("%d(%s)"), KeyIndex, *KeyData.StateName.ToString())
				: FString::Printf(TEXT("%d"), KeyIndex);

			const float TimePixel = TimeToPixelConverter.FrameToPixel(KeyTime);

			const FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Bold", 10);
			const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			FVector2D TextSize = FontMeasureService->Measure(DrawString, SmallLayoutFont);

			static const float TEXT_OFFSET_PX = 10.f;
			bool  bDrawLeft = (Painter.SectionGeometry.Size.X - TimePixel) < (TextSize.X + 22.f) - TEXT_OFFSET_PX;
			float TextPosition = bDrawLeft ? TimePixel - TextSize.X - TEXT_OFFSET_PX : TimePixel + TEXT_OFFSET_PX;

			const float MAJOR_TICK_HEIGHT = 4.0f; 
			FVector2D TextOffset(TextPosition, Painter.SectionGeometry.Size.Y - (MAJOR_TICK_HEIGHT + TextSize.Y));

			const FLinearColor DrawColor = FEditorStyle::GetSlateColor("SelectionColor").GetColor(FWidgetStyle());
			const FVector2D BoxPadding = FVector2D(4.0f, 2.0f);

			const ESlateDrawEffect DrawEffects = Painter.bParentEnabled
				? ESlateDrawEffect::None
				: ESlateDrawEffect::DisabledEffect;

			// draw string
			FSlateDrawElement::MakeBox(
				Painter.DrawElements,
				LayerId + 5,
				Painter.SectionGeometry.ToPaintGeometry(TextOffset - BoxPadding, TextSize + 2.0f * BoxPadding),
				FEditorStyle::GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor::Black.CopyWithNewOpacity(0.5f)
			);

			FSlateDrawElement::MakeText(
				Painter.DrawElements,
				LayerId + 6,
				Painter.SectionGeometry.ToPaintGeometry(TextOffset, TextSize),
				DrawString,
				SmallLayoutFont,
				DrawEffects,
				DrawColor
			);
		}

		return LayerId;
	}

private:

	TWeakPtr<ISequencer> Sequencer;
};

/**
 * static creator
 */
TSharedRef<ISequencerTrackEditor> FStateTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FStateTrackEditor(InSequencer));
}


TSharedRef<ISequencerSection> FStateTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShared<FStateSection>(SectionObject, GetSequencer());
}

FStateTrackEditor::FStateTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{}

/**
 * Add track的入口
 */
void FStateTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	UMovieSceneSequence* RootMovieSceneSequence = GetSequencer()->GetRootMovieSceneSequence();

	if (RootMovieSceneSequence == nullptr)
	{
		return;
	}

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddStateTrack", "State Track"),
		LOCTEXT("AddStateTrackTooltip", "Adds a new track that can define states."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Event"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FStateTrackEditor::HandleAddEventTrackMenuEntryExecute)
		)
	);
}

/**
 * 
 */
void FStateTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
	if (!ObjectClass->IsChildOf(AActor::StaticClass()))
	{
		return;
	}

	//MenuBuilder.AddMenuEntry(
	//	LOCTEXT("AddStateTrack", "State Track Test Menu"),
	//	LOCTEXT("AddStateTrackTooltip", "Adds a new track that can define states."),
	//	FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Event"),
	//	FUIAction()
	//);
}

bool FStateTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return (Type == UMovieSceneStateTrack::StaticClass());
}

bool FStateTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence != nullptr &&
		InSequence->GetClass()->IsChildOf(UStateSequence::StaticClass());
}

const FSlateBrush* FStateTrackEditor:: GetIconBrush() const
{
	return FEditorStyle::GetBrush("Sequencer.Tracks.Event");
}

void FStateTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
	// TODO: add context menu
}

void FStateTrackEditor::HandleAddEventTrackMenuEntryExecute()
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();

	if (FocusedMovieScene == nullptr)
	{
		return;
	}

	UMovieSceneTrack* StateTrack = FocusedMovieScene->FindMasterTrack<UMovieSceneStateTrack>();

	if (StateTrack != nullptr)
	{
		return;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddStateTrack_Transaction", "Add State Track"));
	FocusedMovieScene->Modify();
	
	UMovieSceneStateTrack* NewTrack = FocusedMovieScene->AddMasterTrack<UMovieSceneStateTrack>();
	check(NewTrack);

	UMovieSceneSection* NewSection = NewTrack->CreateNewSection();
	check(NewSection);

	NewTrack->AddSection(*NewSection);
	NewTrack->SetDisplayName(LOCTEXT("TrackName", "State"));

	if (GetSequencer().IsValid())
	{
		GetSequencer()->OnAddTrack(NewTrack);
	}

	GetSequencer()->NotifyMovieSceneDataChanged( EMovieSceneDataChangeType::MovieSceneStructureItemAdded );
}

#undef LOCTEXT_NAMESPACE 
