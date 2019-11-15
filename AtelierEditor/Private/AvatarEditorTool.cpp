// Fill out your copyright notice in the Description page of Project Settings.

#include "AvatarEditorTool.h"
#include "AssetRegistryModule.h"
#include "DataTables/AvatarTransData.h"
#include "Components/SkeletalMeshComponent.h"
#include "ConstructorHelpers.h"
#include "JsonObjectConverter.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "HAL/Platformfilemanager.h"
#include "Editor.h"


// Sets default values
AAvatarEditorTool::AAvatarEditorTool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FAvatarTransData row;
	FTransform CommonTransform = FTransform(FRotator(0, -100, 0), FVector(-130, 0, -140), FVector(1, 1, 1));
	row.Trans = CommonTransform;
	FString Context;
	FString AssetPath = FString("/Game/Data/DataTables/");
	if (LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/CharacterPrefabData.CharacterPrefabData'")))
	{
		DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/CharacterPrefabData.CharacterPrefabData'"));
		DataTable->RowStruct = row.StaticStruct();
	}
	else {
		 
		DataTable = NewObject<UDataTable>(Package, UDataTable::StaticClass(), *FString("CharacterPrefabData"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
		DataTable->RowStruct = row.StaticStruct();
		TMap<FName, FTransform> ConstructMap;
		//Create DataTable
		FAssetRegistryModule::AssetCreated(DataTable);
		DataTable->MarkPackageDirty();

		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("CharacterPrefabData"), *FPackageName::GetAssetPackageExtension());
		UE_LOG(LogTemp, Warning, TEXT("CreateDataTable:%s"), *FilePath);
		bool bSuccess = UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		if (bSuccess)
		{
			UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		}
	}

}

void AAvatarEditorTool::LoadData(int32 CharacterID, FCharacterPrefabData& Data)
{
	LoadTransform = FTransform();
	FString Context;
	DataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/DataTables/CharacterPrefabData.CharacterPrefabData'"));
	if (DataTable) {
		if (CharacterID != 0) {
			if (DataTable->GetRowNames().Num())
			{
				TArray<FName> RowNames = DataTable->GetRowNames();
				if (RowNames.Contains(FName(*FString::FromInt(CharacterID)))) {
					LoadTransform = DataTable->FindRow<FAvatarTransData>(FName(*FString::FromInt(CharacterID)), Context)->Trans;
					
					FAvatarData Avatardata;
					Avatardata.Transform = LoadTransform;
					Avatardata.AnimAsset = DataTable->FindRow<FAvatarTransData>(FName(*FString::FromInt(CharacterID)), Context)->AnimAsset;
					Avatardata.AnimPosition = DataTable->FindRow<FAvatarTransData>(FName(*FString::FromInt(CharacterID)), Context)->AnimPosition;

					Data.CharacterID = CharacterID;
					Data.AvatarData = Avatardata;
				}
			}
		}
	}
}

void AAvatarEditorTool::SaveData(FCharacterPrefabData Data)
{
	FAvatarData AvatarData = Data.AvatarData;
	int32 id = Data.CharacterID;
	CurrTransform = AvatarData.Transform;

	FName CurrNpcIDName = FName(*FString::FromInt(id));
	if (DataTable != nullptr)
	{
		TArray<FAvatarTransData*> OutAllRows;
		FString Context; // Used in error reporting.
		DataTable->GetAllRows<FAvatarTransData>(Context, OutAllRows);
		TArray<FName> key = DataTable->GetRowNames();
		if (key.Contains(CurrNpcIDName)) {
			DataTable->FindRow<FAvatarTransData>(CurrNpcIDName, Context)->Trans = CurrTransform;
			DataTable->FindRow<FAvatarTransData>(CurrNpcIDName, Context)->AnimAsset = AvatarData.AnimAsset;
			DataTable->FindRow<FAvatarTransData>(CurrNpcIDName, Context)->AnimPosition = AvatarData.AnimPosition;
		}
		else
		{
			FAvatarTransData row;
			row.Trans = CurrTransform;
			row.AnimAsset = AvatarData.AnimAsset;
			row.AnimPosition = AvatarData.AnimPosition;

			DataTable->AddRow(CurrNpcIDName, row);
		}
		DataTable->MarkPackageDirty();
		FString AssetPath = FString("/Game/Data/DataTables/");
		FString FilePath = FString::Printf(TEXT("%s%s%s"), *AssetPath, *FString("CharacterPrefabData"), *FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, DataTable, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
	}
}

void AAvatarEditorTool::SelectComp()
{
	if (Char != nullptr) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(Char, true, true);
		GEditor->SelectComponent(Char->GetMesh(), true, true);
	}
	
}

bool AAvatarEditorTool::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverWriting)
{
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	if (!AllowOverWriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirectory))
		{
			return false;
		}
	}
	
	FString FinalString = "";
	for (FString& Each : SaveText)
	{
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}

	return FFileHelper::SaveStringToFile(FinalString, *SaveDirectory);
}


void AAvatarEditorTool::SaveCharacterPrefab(FCharacterPrefabData Data)
{
	
}





