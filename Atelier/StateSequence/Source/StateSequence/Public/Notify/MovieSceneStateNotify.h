// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneStateNotify.generated.h"

UCLASS(abstract, Blueprintable, const, hidecategories = Object, collapsecategories)
class STATESEQUENCE_API UMovieSceneStateNotify : public UObject
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintNativeEvent)
	FString GetNotifyName() const;

	UFUNCTION(BlueprintImplementableEvent)
	bool Received_Notify(AActor* owner) const;

	virtual void ValidateAssociatedAssets() {}

	virtual void Notify(AActor* owner);

#if WITH_EDITORONLY_DATA
	/** Color of Notify in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	FColor NotifyColor;
#endif // WITH_EDITORONLY_DATA

	// @todo document 
	virtual FLinearColor GetEditorColor()
	{
#if WITH_EDITORONLY_DATA
		return FLinearColor(NotifyColor);
#else
		return FLinearColor::Black;
#endif // WITH_EDITORONLY_DATA
	}

	/** UObject Interface */
	virtual void PostLoad() override;
	virtual void PreSave(const class ITargetPlatform* TargetPlatform) override;
};
