// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-07-16
#include "SkillPreviewSceneCustomization.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Widgets/Input/STextComboBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "EditorStyleSet.h"
#include "DetailWidgetRow.h"
#include "Common/GADataTable.h"
#include "SkillEditorProxy.h"
#include "Widgets/SCharaSelectionWidget.h"
#include "Widgets/SWeaponSelectionWidget.h"

#define LOCTEXT_NAMESPACE "AnimNodeSlotDetails"

FSkillPreviewSceneDetailCustomization::FSkillPreviewSceneDetailCustomization(class USkillEditorProxy* SkillEditorProxyPtr)
	: SkillEditorPtr(SkillEditorProxyPtr)
{
}

FSkillPreviewSceneDetailCustomization::~FSkillPreviewSceneDetailCustomization()
{
	SkillEditorPtr = nullptr;
}

void FSkillPreviewSceneDetailCustomization::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& PrevCharacterGroup = DetailBuilder.EditCategory(TEXT("Config"));

	TSharedPtr<SCharaSelectionWidget> PrevCharaSelWidget;
	PrevCharacterGroup.AddCustomRow(LOCTEXT("SkillEditor_Character", "Character: "))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("SkillEditor_Character", "Character: "))
		]
		.ValueContent()
		.MinDesiredWidth(125.f * 3.f)
		.MaxDesiredWidth(125.f * 3.f)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PrevCharaSelWidget, SCharaSelectionWidget)
					.OnSelectionChanged_Lambda([this](FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
					{
						if (InSelectedItem.IsValid())
							SkillEditorPtr->ChangePrevCharacter(InSelectedItem->GetCharacterId());
					})
			]

		];
	PrevCharaSelWidget->SetCurrSelection(SkillEditorPtr->PrevCharacterId);

	// Weapon Selection
	TSharedPtr<SWeaponSelectionWidget> CharaWeaponSelWidget;
	{
		PrevCharacterGroup.AddCustomRow(LOCTEXT("SkillEditor_Character_Weapon", "Character Weapon: "))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("SkillEditor_Character_Weapon", "Character Weapon: "))
			]
			.ValueContent()
			.MinDesiredWidth(125.f * 3.f)
			.MaxDesiredWidth(125.f * 3.f)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(CharaWeaponSelWidget, SWeaponSelectionWidget)
						// .OnSelectionChanged_Lambda([this](FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
						// {
						// 	if (InSelectedItem.IsValid())
						// 		SkillEditorPtr->ChangePrevCharacter(InSelectedItem->GetCharacterId());
						// })
				]

			];
		// CharaWeaponSelWidget->SetCurrSelection(SkillEditorPtr->PrevCharacterId);
	}
	
	TSharedPtr<SCharaSelectionWidget> PrevEnemySelWidget;
	PrevCharacterGroup.AddCustomRow(LOCTEXT("SkillEditor_Stake", "Stake: "))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("SkillEditor_Stake", "Stake: "))
		]
		.ValueContent()
		.MinDesiredWidth(125.f * 3.f)
		.MaxDesiredWidth(125.f * 3.f)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(PrevEnemySelWidget, SCharaSelectionWidget)
					.OnSelectionChanged_Lambda([this](FCharaEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo) 
					{
						if (InSelectedItem.IsValid())
							SkillEditorPtr->ChangePrevEnemy(InSelectedItem->GetCharacterId());
					})
			]
		];
	PrevEnemySelWidget->SetCurrSelection(SkillEditorPtr->PrevEnemyId);
}

#undef LOCTEXT_NAMESPACE