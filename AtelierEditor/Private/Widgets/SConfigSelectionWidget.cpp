#include "Widgets/SConfigSelectionWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "StageEditor/Public/StageEditorSettings.h"
#include "FileManager.h"
#include "SButton.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "FileHelper.h"

#define LOCTEXT_NAMESPACE "ConfigSelectionWidget"

FPreviewConfigEntry::FPreviewConfigEntry(FString Name, FString Path)
{
	this->Name = Name;
	this->Path = Path;
}

void SConfigSelectionWidget::Construct(const FArguments & InArgs)
{
	OnSelectionChanged = InArgs._OnSelectionChanged;

	// Collection data
	CollectionWeatherEntry();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ConfigComboBox, SConfigComboBox)
			.OptionsSource(&ConfigEntryArray)
			.OnGenerateWidget(this, &SConfigSelectionWidget::MakeEntryWidget)
			.OnSelectionChanged(this, &SConfigSelectionWidget::OnComboBoxSelectionChanged)
			.OnFilterText(this, &SConfigSelectionWidget::OnFilterTextChanged)
			[
				SNew(STextBlock)
				.Text(this, &SConfigSelectionWidget::GetCurrSelectionText)
			]
		]
	];
	
	ConfigComboBox->AddHeadSlot
	(
		SNew(SButton)
		.Text(FText::FromName("New File"))
		.OnClicked(this, &SConfigSelectionWidget::NewConfigFile)
	);

	if (FConfigEntryPtr* InitSelection = ConfigEntryArray.FindByPredicate([&](FConfigEntryPtr& Item)
	{
		return Item->GetPath() == InArgs._InitSelectedConfigPath.Get();
	}))
	{
		ConfigComboBox->SetSelectedItem(*InitSelection);
	}
}

void SConfigSelectionWidget::SetCurrSelection(FString ConfigPath)
{
	for (auto EntryPtr : ConfigEntryArray)
	{
		if (EntryPtr->GetPath() == ConfigPath)
		{
			CurrSelection = EntryPtr;
			break;
		}
	}
}

void SConfigSelectionWidget::CollectionWeatherEntry(const FString & FilterString)
{
	ConfigEntryArray.Empty();
	FString Folder = FPaths::GetPath(GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath);
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Folder, _T("*.socf"), true, false, false);
	for (int i = 0; i < Files.Num(); i++) 
	{
		FString Name = Files[i].Replace(*FPaths::GetPath(Files[i]), _T("")).Replace(_T(".socf"), _T("")).Replace(_T("/"), _T(""));
		FString Path = Files[i].Replace(*Folder, _T(""));
		ConfigEntryArray.Add(MakeShareable(new FPreviewConfigEntry(Name, Path)));
	}
}

TSharedRef<class SWidget> SConfigSelectionWidget::MakeEntryWidget(FConfigEntryPtr InEntry) const
{
	return SNew(STextBlock)
		.MinDesiredWidth(300.f)
		.Text(InEntry->GetEntryText());
}

void SConfigSelectionWidget::OnComboBoxSelectionChanged(FConfigEntryPtr InSelectedItem, ESelectInfo::Type SelectInfo)
{
	CurrSelection = InSelectedItem;
	OnSelectionChanged.ExecuteIfBound(InSelectedItem, SelectInfo);
}

FText SConfigSelectionWidget::GetCurrSelectionText() const
{
	if (CurrSelection.IsValid())
		return CurrSelection->GetEntryText();
	return FText();
}

void SConfigSelectionWidget::OnFilterTextChanged(const FText & InFilterText)
{
	CollectionWeatherEntry(InFilterText.ToString());
	ConfigComboBox->RefreshOptions();
}

FReply SConfigSelectionWidget::NewConfigFile()
{
	auto PickConfigPath = [](const FString& Title) {
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString Folder = FPaths::GetPath(GetDefault<UStageEditorSettings>()->StageDataFilePath.FilePath);
		FString CurrentFilename;

		const FString FileTypes = TEXT("Config SOCF (*.socf)|*.socf");
		TArray<FString> OutFilenames;
		DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			Title,
			Folder,
			(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".socf"),
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);
		if (OutFilenames.Num() > 0)
		{
			return OutFilenames[0];
		}

		return FString();
	};
	FString FilePath = PickConfigPath(FString::Printf(TEXT("Save Config as SOCF... ")));
	FFileHelper::SaveStringToFile(FString(), *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), EFileWrite::FILEWRITE_NoFail);
	CollectionWeatherEntry();
	ConfigComboBox->RefreshOptions();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
