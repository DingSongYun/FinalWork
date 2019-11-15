// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-16

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SComboBox.h"
#include "IDetailCustomization.h"

struct FPreviewCharacterEntry;

class FSkillPreviewSceneDetailCustomization : public IDetailCustomization
{
public:
	FSkillPreviewSceneDetailCustomization(class USkillEditorProxy* SkillEditorProxyPtr);

	~FSkillPreviewSceneDetailCustomization();
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;

private:
	class USkillEditorProxy* SkillEditorPtr;

	TArray<TSharedPtr<FPreviewCharacterEntry>> CharacterEntry;
};