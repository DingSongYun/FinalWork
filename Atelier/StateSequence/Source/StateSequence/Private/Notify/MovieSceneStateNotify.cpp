// Fill out your copyright notice in the Description page of Project Settings.


#include "MovieSceneStateNotify.h"

UMovieSceneStateNotify::UMovieSceneStateNotify(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 200, 200, 255);
#endif // WITH_EDITORONLY_DATA
}

void UMovieSceneStateNotify::Notify(AActor* owner)
{
	Received_Notify(owner);
}

FString UMovieSceneStateNotify::GetNotifyName_Implementation() const
{
	UObject* ClassGeneratedBy = GetClass()->ClassGeneratedBy;
	FString NotifyName;

	if (ClassGeneratedBy)
	{
		// GeneratedBy will be valid for blueprint types and gives a clean name without a suffix
		NotifyName = ClassGeneratedBy->GetName();
	}
	else
	{
		// Native notify classes are clean without a suffix otherwise
		NotifyName = GetClass()->GetName();
	}

	NotifyName = NotifyName.Replace(TEXT("_C"), TEXT(""), ESearchCase::CaseSensitive);

	return NotifyName;
}

void UMovieSceneStateNotify::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	// Ensure that all loaded notifies are transactional
	SetFlags(GetFlags() | RF_Transactional);

	// Make sure the asset isn't bogus (e.g., a looping particle system in a one-shot notify)
	ValidateAssociatedAssets();
#endif
}

void UMovieSceneStateNotify::PreSave(const class ITargetPlatform* TargetPlatform)
{
#if WITH_EDITOR
	ValidateAssociatedAssets();
#endif
	Super::PreSave(TargetPlatform);
}
