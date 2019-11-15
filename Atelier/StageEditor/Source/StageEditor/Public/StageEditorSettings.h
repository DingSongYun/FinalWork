#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "StageEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "StageEditor"))
class STAGEEDITOR_API UStageEditorSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = Editing)
	FFilePath 					StageDataFilePath;
};