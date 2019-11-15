#pragma once

#include "ActionEvent.h"
#include "ActionSkillTypes.generated.h"

/*********************************************************************/
// FActionKeyFrame
/*********************************************************************/
USTRUCT(BlueprintType)
struct ACTIONSKILL_API FActionKeyFrame
{
    GENERATED_BODY()
public:
    FActionKeyFrame();

    // void onKeyFrameReached() {}

    FActionEventPtr GetEvent() const;
public:
    UPROPERTY(EditAnywhere, Category = "KeyFrame")
    FName Name;

    UPROPERTY(EditAnywhere, Category = "KeyFrame")
    float Time;

    UPROPERTY(EditAnywhere, Category = "KeyFrame")
    int32 EventId;

    UPROPERTY(EditAnywhere, Category = "KeyEventArgs")
    FUserStructOnScope EventArgs;
};

/*********************************************************************/
// FAction
/*********************************************************************/
USTRUCT(BlueprintType)
struct ACTIONSKILL_API FAction
{
    GENERATED_BODY()
public:
    friend bool operator==(const FAction& Lhs, const FAction& Rhs)
    {
        return &Lhs == &Rhs;
    }

    float GetActionLength();
    float GetActionFrame();
public:
    UPROPERTY(EditAnywhere, Category = "Action")
    FName AnimName;

    UPROPERTY(EditAnywhere, Category="Action")
    TSoftObjectPtr<class UAnimSequenceBase> AnimReference;

    UPROPERTY(EditAnywhere, Category = "Action")
    TArray<FActionKeyFrame> Keys;
};

/*********************************************************************/
// FActionSkillTemplate
/*********************************************************************/
USTRUCT()
struct ACTIONSKILL_API FActionSkillTemplate
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Skill")
    uint32 Id;

    UPROPERTY(EditAnywhere, Category="Skill")
    FString Name;

    UPROPERTY(EditAnywhere, Category="Skill")
    TArray<FAction> Actions;
};

USTRUCT()
struct ACTIONSKILL_API FActionEventTemplate
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Event")
	uint32 Id;

	UPROPERTY(EditAnywhere, Category = "Event")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "Event")
	TArray<FAction> Actions;
};