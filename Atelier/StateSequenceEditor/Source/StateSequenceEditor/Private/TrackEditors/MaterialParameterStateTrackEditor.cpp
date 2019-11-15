// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MaterialParameterStateTrackEditor.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Materials/MaterialParameterCollection.h"
#include "Sections/MovieSceneParameterSection.h"
#include "SequencerUtilities.h"
#include "Algo/Sort.h"
#include "Styling/SlateIconFinder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"

#include "ISequencerSection.h"

#define LOCTEXT_NAMESPACE "MaterialParameterStateTrackEditor"

/**
 * A movie scene section for material parameters.
 */
class FStateParameterSection
	: public FSequencerSection
{
public:

	FStateParameterSection(UMovieSceneSection& InSectionObject)
		: FSequencerSection(InSectionObject)
	{ }

public:

	//~ ISequencerSection interface

	virtual bool RequestDeleteCategory(const TArray<FName>& CategoryNamePath) override 
	{
		const FScopedTransaction Transaction(LOCTEXT("DeleteVectorOrColorParameter", "Delete vector or color parameter"));
		UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(WeakSection.Get());
		if (ParameterSection->Modify())
		{
			bool bVectorParameterDeleted = ParameterSection->RemoveVectorParameter(CategoryNamePath[0]);
			bool bColorParameterDeleted = ParameterSection->RemoveColorParameter(CategoryNamePath[0]);
			return bVectorParameterDeleted || bColorParameterDeleted;
		}
		return false;
	}

	virtual bool RequestDeleteKeyArea(const TArray<FName>& KeyAreaNamePath) override 
	{
		if (KeyAreaNamePath.Num() == 1)
		{
			const FScopedTransaction Transaction(LOCTEXT("DeleteScalarParameter", "Delete scalar parameter"));
			UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(WeakSection.Get());
			if (ParameterSection->TryModify())
			{
				return ParameterSection->RemoveScalarParameter(KeyAreaNamePath[0]);
			}
		}
		return false;
	}
};

FMaterialParameterStateTrackEditor::FMaterialParameterStateTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

TSharedRef<ISequencerTrackEditor> FMaterialParameterStateTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
{
	return MakeShared<FMaterialParameterStateTrackEditor>(OwningSequencer);
}

TSharedRef<ISequencerSection> FMaterialParameterStateTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(&SectionObject);
	checkf(ParameterSection != nullptr, TEXT("Unsupported section type."));
	
	return MakeShareable(new FStateParameterSection(*ParameterSection));
}

TSharedRef<SWidget> CreateAssetPicker(FOnAssetSelected OnAssetSelected, FOnAssetEnterPressed OnAssetEnterPressed)
{
	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.OnAssetSelected = OnAssetSelected;
		AssetPickerConfig.OnAssetEnterPressed = OnAssetEnterPressed;
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.Filter.ClassNames.Add(UMaterialParameterCollection::StaticClass()->GetFName());
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	return SNew(SBox)
		.WidthOverride(300.0f)
		.HeightOverride(300.f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

void FMaterialParameterStateTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
	UMovieSceneMaterialParameterCollectionTrack* MPCTrack = Cast<UMovieSceneMaterialParameterCollectionTrack>(Track);

	auto AssignAsset = [MPCTrack](const FAssetData& InAssetData)
	{
		UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(InAssetData.GetAsset());

		if (MPC)
		{
			FScopedTransaction Transaction(LOCTEXT("SetAssetTransaction", "Assign Material Parameter Collection"));
			MPCTrack->Modify();
			MPCTrack->SetDisplayName(FText::FromString(MPC->GetName()));
			MPCTrack->MPC = MPC;
		}

		FSlateApplication::Get().DismissAllMenus();
	};

	auto AssignAssetEnterPressed = [AssignAsset](const TArray<FAssetData>& InAssetData)
	{
		if (InAssetData.Num() > 0)
		{
			AssignAsset(InAssetData[0].GetAsset());
		}
	};

	auto SubMenuCallback = [this, AssignAsset, AssignAssetEnterPressed](FMenuBuilder& SubMenuBuilder)
	{
		SubMenuBuilder.AddWidget(CreateAssetPicker(FOnAssetSelected::CreateLambda(AssignAsset), FOnAssetEnterPressed::CreateLambda(AssignAssetEnterPressed)), FText::GetEmpty(), true);
	};

	MenuBuilder.AddSubMenu(
		LOCTEXT("SetAsset", "Set Asset"),
		LOCTEXT("SetAsset_ToolTip", "Sets the Material Parameter Collection that this track animates."),
		FNewMenuDelegate::CreateLambda(SubMenuCallback)
	);
}


void FMaterialParameterStateTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	auto SubMenuCallback = [this](FMenuBuilder& SubMenuBuilder)
	{
		SubMenuBuilder.AddWidget(CreateAssetPicker(FOnAssetSelected::CreateRaw(this, &FMaterialParameterStateTrackEditor::AddTrackToSequence), FOnAssetEnterPressed::CreateRaw(this, &FMaterialParameterStateTrackEditor::AddTrackToSequenceEnterPressed)), FText::GetEmpty(), true);
	};


	MenuBuilder.AddSubMenu(
		LOCTEXT("AddMPCTrack", "Material Parameter Collection Track"),
		LOCTEXT("AddMPCTrackToolTip", "Adds a new track that controls parameters within a Material Parameter Collection."),
		FNewMenuDelegate::CreateLambda(SubMenuCallback),
		false,
		FSlateIconFinder::FindIconForClass(UMaterialParameterCollection::StaticClass())
	);
}

void FMaterialParameterStateTrackEditor::AddTrackToSequence(const FAssetData& InAssetData)
{
	FSlateApplication::Get().DismissAllMenus();

	UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(InAssetData.GetAsset());
	UMovieScene* MovieScene = GetFocusedMovieScene();
	if (!MPC || !MovieScene)
	{
		return;
	}

	if (MovieScene->IsReadOnly())
	{
		return;
	}

	// Attempt to find an existing MPC track that animates this object
	for (UMovieSceneTrack* Track : MovieScene->GetMasterTracks())
	{
		if (auto* MPCTrack = Cast<UMovieSceneMaterialParameterCollectionTrack>(Track))
		{
			if (MPCTrack->MPC == MPC)
			{
				return;
			}
		}
	}

	const FScopedTransaction Transaction(LOCTEXT("AddTrackDescription", "Add Material Parameter Collection Track"));

	MovieScene->Modify();
	UMovieSceneMaterialParameterCollectionTrack* Track = MovieScene->AddMasterTrack<UMovieSceneMaterialParameterCollectionTrack>();
	check(Track);

	UMovieSceneSection* NewSection = Track->CreateNewSection();
	check(NewSection);

	Track->AddSection(*NewSection);
	Track->MPC = MPC;
	Track->SetDisplayName(FText::FromString(MPC->GetName()));

	if (GetSequencer().IsValid())
	{
		GetSequencer()->OnAddTrack(Track);
	}
	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

void FMaterialParameterStateTrackEditor::AddTrackToSequenceEnterPressed(const TArray<FAssetData>& InAssetData)
{
	if (InAssetData.Num() > 0)
	{
		AddTrackToSequence(InAssetData[0].GetAsset());
	}
}

bool FMaterialParameterStateTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneMaterialParameterCollectionTrack::StaticClass();
}

bool FMaterialParameterStateTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	static UClass* StateSequenceClass = FindObject<UClass>(ANY_PACKAGE, TEXT("StateSequence"), true);
	return InSequence != nullptr &&
		((StateSequenceClass != nullptr && InSequence->GetClass()->IsChildOf(StateSequenceClass)));
}

const FSlateBrush* FMaterialParameterStateTrackEditor::GetIconBrush() const
{
	return FSlateIconFinder::FindIconForClass(UMaterialParameterCollection::StaticClass()).GetIcon();
}

TSharedPtr<SWidget> FMaterialParameterStateTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	UMovieSceneMaterialParameterCollectionTrack* MPCTrack = Cast<UMovieSceneMaterialParameterCollectionTrack>(Track);
	FOnGetContent MenuContent = FOnGetContent::CreateSP(this, &FMaterialParameterStateTrackEditor::OnGetAddParameterMenuContent, MPCTrack);

	return FSequencerUtilities::MakeAddButton(LOCTEXT("AddParameterButton", "Parameter"), MenuContent, Params.NodeIsHovered, GetSequencer());
}

TSharedRef<SWidget> FMaterialParameterStateTrackEditor::OnGetAddParameterMenuContent(UMovieSceneMaterialParameterCollectionTrack* MPCTrack)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ScalarParametersHeading", "Scalar"));
	{
		TArray<FCollectionScalarParameter> ScalarParameters = MPCTrack->MPC->ScalarParameters;
		Algo::SortBy(ScalarParameters, &FCollectionParameterBase::ParameterName);

		for (const FCollectionScalarParameter& Scalar : ScalarParameters)
		{
			MenuBuilder.AddMenuEntry(
				FText::FromName(Scalar.ParameterName),
				FText(),
				FSlateIcon(),
				FExecuteAction::CreateSP(this, &FMaterialParameterStateTrackEditor::AddScalarParameter, MPCTrack, Scalar)
			);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("VectorParametersHeading", "Vector"));
	{
		TArray<FCollectionVectorParameter> VectorParameters = MPCTrack->MPC->VectorParameters;
		Algo::SortBy(VectorParameters, &FCollectionParameterBase::ParameterName);

		for (const FCollectionVectorParameter& Vector : VectorParameters)
		{
			MenuBuilder.AddMenuEntry(
				FText::FromName(Vector.ParameterName),
				FText(),
				FSlateIcon(),
				FExecuteAction::CreateSP(this, &FMaterialParameterStateTrackEditor::AddVectorParameter, MPCTrack, Vector)
			);
		}
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}


void FMaterialParameterStateTrackEditor::AddScalarParameter(UMovieSceneMaterialParameterCollectionTrack* Track, FCollectionScalarParameter Parameter)
{
	if (!Track->MPC)
	{
		return;
	}

	FFrameNumber KeyTime = GetTimeForKey();

	const FScopedTransaction Transaction(LOCTEXT("AddScalarParameter", "Add scalar parameter"));
	Track->Modify();
	Track->AddScalarParameterKey(Parameter.ParameterName, KeyTime, Parameter.DefaultValue);
	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}


void FMaterialParameterStateTrackEditor::AddVectorParameter(UMovieSceneMaterialParameterCollectionTrack* Track, FCollectionVectorParameter Parameter)
{
	if (!Track->MPC)
	{
		return;
	}

	FFrameNumber KeyTime = GetTimeForKey();

	const FScopedTransaction Transaction(LOCTEXT("AddVectorParameter", "Add vector parameter"));
	Track->Modify();
	Track->AddColorParameterKey(Parameter.ParameterName, KeyTime, Parameter.DefaultValue);
	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

bool FMaterialParameterStateTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(Asset);
	if (MPC)
	{
		AddTrackToSequence(FAssetData(MPC));
	}

	return MPC != nullptr;
}

#undef LOCTEXT_NAMESPACE
