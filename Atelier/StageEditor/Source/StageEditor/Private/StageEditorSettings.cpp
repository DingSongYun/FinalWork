#include "StageEditorSettings.h"

UStageEditorSettings::UStageEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = TEXT("Atelier");
	SectionName = TEXT("StageEditor");
	StageDataFilePath.FilePath = TEXT("");
}