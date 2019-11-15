#include "StagePlacementCategory.h"
#include "Editor.h"
#include "AssetRegistryModule.h"
#include "StageObjectFactory.h"
#include "PlacementCategory.h"
#include "Common/GADataTable.h"

IAssetRegistry& GetAssetRegistry()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    return AssetRegistryModule.Get();
}

USActorFactoryCharacter::USActorFactoryCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NewActorClass = APlacementCharacterSpawner::StaticClass();
}

FName UTblConfiguredFactory::GetTableName() const
{
    return FName("Npc");
}

void UTblConfiguredFactory::GetTableData(TArray<UGADataRow*>& OutData) const
{
    struct TableData
    {
        TableData(FName TblName)
        {
            UGADataTable* NpcTable = UGADataTable::GetDataTable(TblName.ToString());
            NpcTable->GetAllRows(Rows);
        }

        TArray<UGADataRow*> Rows;
    };
    static const TableData NpcData(GetTableName());
    for (UGADataRow* DataRow : NpcData.Rows)
    {
        int32 NpcType = DataRow->GetNumber("Type");
        if (CanSpawnWithType(NpcType)) OutData.Add(DataRow);
    }
}

void UTblConfiguredFactory::CollectPlacements(TArray<TSharedPtr<FPlacementItem>>& OutSet) const
{
    // FAssetData to spawn
    const FAssetData& AssetData = GetAsset();

    // Table config data
    TArray<UGADataRow*> TblData;
    GetTableData(TblData);

    OutSet.Empty();
    for (UGADataRow* DataRow : TblData)
    {
        USActorFactoryCharacter* Factory = NewObject<USActorFactoryCharacter>(const_cast<UTblConfiguredFactory*>(this));
        // UActorFactory* DefaultFactory = GEditor->FindActorFactoryByClassForActorClass(UActorFactory::StaticClass(), AssetData.GetClass());
        Factory->AddToRoot();
        Factory->SetCharacterId(DataRow->GetNumber("Id"));
        TSharedPtr<FPlacementItem> Item = MakeShareable(new FPlacementItem(Factory, AssetData));
        Item->DisplayName = FText::FromString(FString::Printf(TEXT("%s(%d)"), *DataRow->GetStr("NameZh"), DataRow->GetNumber("Id")));
        OutSet.Add(Item);
    }
}

FAssetData UCharacterFactory::GetAsset() const
{
    static FAssetData AssetData = GetAssetRegistry().GetAssetByObjectPath(
                    "/StageEditor/StageObjects/CharacterSpawner.CharacterSpawner" );
    return AssetData;
}

FAssetData UMonsterFactory::GetAsset() const
{
    static FAssetData AssetData = GetAssetRegistry().GetAssetByObjectPath(
                    "/StageEditor/StageObjects/CharacterSpawner.CharacterSpawner" );
    return AssetData;
}

FAssetData UCollectionFactory::GetAsset() const
{
    static FAssetData AssetData = GetAssetRegistry().GetAssetByObjectPath(
                    "/StageEditor/StageObjects/CharacterSpawner.CharacterSpawner" );
    return AssetData;
}

FAssetData UGearFactory::GetAsset() const
{
    static FAssetData AssetData = GetAssetRegistry().GetAssetByObjectPath(
                    "/StageEditor/StageObjects/CharacterSpawner.CharacterSpawner" );
    return AssetData;
}