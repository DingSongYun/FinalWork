// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStateSequenceEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterAssetTools();
	void UnregisterAssetTools();
	void RegisterAssetTypeAction(class IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action);

private:
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TSharedPtr<class FStateSequenceEditorTabBinding> BlueprintEditorTabBinding;

	//~ Begin: Sequence Track Editor
	void RegisterSequenceTrackEditor();
	void UnregisterSequenceTrackEditor();
	//~ End: Sequence Track Editor

private:
	TArray<FDelegateHandle> SequenceTrackCreateEditorHandles;
};
