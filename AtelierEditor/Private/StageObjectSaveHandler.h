// Copyright 2018 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2018-06-29

#pragma once
#include "Model/MapDataModel.h"
#include "Stage/StageObject.h"
#include "Stage/MonsterSpawn.h"
#include "Stage/PlayerSpawn.h"
#include "Stage/ExitArea.h"
#include "Stage/SpawnGroup.h"
#include "Stage/SmartNavLinkProxy.h"
#include "Character/BaseCharacter.h"

class FStageObjectSaveHandler
{
private:
	static void DumpBaseSaveData(FStageSaveData& OutSaveData, IStageObject* Actor)
	{
		if (Actor == nullptr)
		{
			return ;
		}

		Actor->Dump(OutSaveData);
	}

	static void DumpMonsterSpawnData(FMapDataModel& OutMapData, AMonsterSpawn* Actor)
	{
		if (Actor == nullptr)
		{
			return ;
		}

		FMapMonsterSpawnData SaveData; 
		DumpBaseSaveData(SaveData, Actor);
		SaveData.MonsterGroupId = Actor->MonsterGroupId;
		SaveData.MonsterId = Actor->MonsterId;
		SaveData.DefaultAction = Actor->DefaultAction;
		SaveData.Radius = Actor->GetArea();
		SaveData.bDoNavFail = Actor->NeedDoNavFail();
		SaveData.RetrieveTime = Actor->RetrieveTime;
		SaveData.HuntArea = Actor->HuntArea;
		SaveData.CautionArea = Actor->CautionArea;
		SaveData.GoBackDistance = Actor->GoBackDistance;
		SaveData.PatrolArea = Actor->PatrolArea;
		SaveData.MinPatrolFrequence = Actor->MinPatrolFrequence;
		SaveData.MaxPatrolFrequence = Actor->MaxPatrolFrequence;
		SaveData.isDefaultSpawn = Actor->isDefaultSpawn;
		SaveData.UniqueId = Actor->UniqueId;
		SaveData.VisibilitySlot = Actor->VisibilitySlot;
		SaveData.DefaultStatus = Actor->DefaultStatus;
		SaveData.isUndeadGather = Actor->isUndeadGather;
		SaveData.ActorRef = Actor->ActorRef;
		FTransform Transform = Actor->GetTransform();
		SaveData.PerformPatrolName = Actor->PerformPatrolName;
		SaveData.VisableStatus = (int)Actor->VisableStatus;
		SaveData.BoxId = Actor->BoxId;
		SaveData.ResurrectionTimes = Actor->ResurrectionTimes;

		for(FAreaNode& Node : Actor->PatrolPath)
		{
			SaveData.PatrolPath.Add(Node.GetWorldLocation(Transform));
		}
		for (int i = 0; i < Actor->FeatureList.Num(); i++) {
			if ((int)Actor->FeatureList[i]) {
				SaveData.FeatureList.Add((int)Actor->FeatureList[i]);
			}
		}

		OutMapData.MonsterSpawnPoints.Add(SaveData);
	}

	static void DumpPlayerSpawnData(FMapDataModel& OutMapData, APlayerSpawn* Actor)
	{
		if (Actor == nullptr)
		{
			return ;
		}

		FMapPlayerSpawnData SaveData; 
		DumpBaseSaveData(SaveData, Actor);

		SaveData.Radius = Actor->GetArea();
		SaveData.SpawnName = Actor->SpawnName;
		SaveData.IsDefault = Actor->IsDefault;

		OutMapData.PlayerSpawnPoint.Add(SaveData);
	}

	static void DumpExitAreaData(FMapDataModel& OutMapData, AExitArea* Actor)
	{
		if (Actor == nullptr)
		{
			return ;
		}

		FMapExitAreaData SaveData;
		Actor->Dump(SaveData);
		SaveData.ToMapId = Actor->ToMapId;
		SaveData.Radius = Actor->GetArea();
		SaveData.ToSpawnName = Actor->ToSpawnName;
		//DumpBaseSaveData(SaveData, Actor);
		//OutMapData.ExitArea.NextMapId = FString();
		OutMapData.ExitAreas.Add(SaveData);
	}

	static void DumpSpawnGroupData(FMapDataModel& OutMapData, ASpawnGroup* Actor)
	{
		if (Actor == nullptr)
		{
			return;
		}

		FMapSpawnGroupData SaveData;
		Actor->Dump(SaveData);
		SaveData.MonsterGroupId = Actor->MonsterGroupId;
		SaveData.MonsterSpawnCount = Actor->MonsterSpawnCount;
		SaveData.GroupRetrieveTime = Actor->GroupRetrieveTime;
		SaveData.GroupMinRetrieveTime = Actor->GroupMinRetrieveTime;
		SaveData.groupPatrolArea = Actor->groupPatrolArea;
		SaveData.isUndeadGatherGroup = Actor->isUndeadGatherGroup;
		SaveData.isDefaultSpawn = Actor->isDefaultSpawn;
		SaveData.VisableStatus = (int)Actor->VisableStatus;
		SaveData.ResurrectionTimes = Actor->ResurrectionTimes;
		OutMapData.SpawnGroup.Add(SaveData);
	}

	static void DumpMapLinks(FMapDataModel& OutMapData, ASmartNavLinkProxy* Actor)
	{
		if (Actor == nullptr)
		{
			return;
		}

		FMapLinkData SaveData;
		Actor->Dump(SaveData);
		OutMapData.MapLinks.Add(SaveData);
	}

public:
	static void DumpStageObject(FMapDataModel& OutMapData, IStageObject* StageObject)
	{
		if (StageObject == nullptr)
		{
			return;
		}

		EStageObjectType Type = StageObject->GetSObjType();
		switch (StageObject->GetSObjType())
		{
		case EStageObjectType::MonsterSpawn:
			DumpMonsterSpawnData(OutMapData, dynamic_cast<AMonsterSpawn *>(StageObject));
			break;
		case EStageObjectType::PlayerSpawn:
			DumpPlayerSpawnData(OutMapData, dynamic_cast<APlayerSpawn*>(StageObject));
			break;
		case EStageObjectType::ExitArea:
			DumpExitAreaData(OutMapData, dynamic_cast<AExitArea*>(StageObject));
			break;
		case EStageObjectType::Building:
			break;
		case EStageObjectType::SpawnGroup:
			DumpSpawnGroupData(OutMapData, dynamic_cast<ASpawnGroup*>(StageObject));
			break;
		case EStageObjectType::SmartNavLink:
			//DumpMapLinks(OutMapData, dynamic_cast<ASmartNavLinkProxy*>(StageObject));
			break;
		default:
			return ;
		}
	}
};
