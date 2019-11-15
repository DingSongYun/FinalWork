#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LuaBlueprintLibrary.h"

#include "EditorStaticLibrary.generated.h"

UCLASS()
class ATELIEREDITOR_API UEditorStaticLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	static class UEditorEngine* GetEditorEngine();

	UFUNCTION(BlueprintCallable, CallInEditor)
	static class UWorld* GetEditorWorld();

#pragma region SLua
	UFUNCTION(BlueprintCallable, CallInEditor, meta=( DisplayName="Ed Call To Lua With Arguments" ), Category="slua")
	static FLuaBPVar CallToLuaWithArgs(FString FunctionName,const TArray<FLuaBPVar>& Args)
	{
		return ULuaBlueprintLibrary::CallToLuaWithArgs(FunctionName, Args, "");
	}

	UFUNCTION(BlueprintCallable, CallInEditor, meta=( DisplayName="Ed Call To Lua" ), Category="slua")
	static FLuaBPVar CallToLua(FString FunctionName)
	{
		return ULuaBlueprintLibrary::CallToLua(FunctionName, "");
	}
#pragma endregion

#pragma region Viewport
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void PossessViewportToActor(class AActor* InActor);

	static void LockViewportToActor(class FLevelEditorViewportClient* InViewportClient, class AActor* InActor);

	UFUNCTION(BlueprintCallable, CallInEditor)
	static void UnpossessViewportFromActor(class AActor* InActor);

	UFUNCTION(BlueprintCallable, CallInEditor)
	static void SetViewportLocation(const FVector& Location);
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void SetViewportRotation(const FRotator& Rotation);
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void SetViewportFOV(float fov);

	UFUNCTION(BlueprintCallable, CallInEditor)
	static FVector GetViewportLocation(const FVector& Location);
	UFUNCTION(BlueprintCallable, CallInEditor)
	static FRotator GetViewportRotation(const FRotator& Rotation);
	UFUNCTION(BlueprintCallable, CallInEditor)
	static float GetViewportFOV(float fov);
#pragma endregion

};
