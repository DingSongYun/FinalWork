// Copyright

#include "EditorMenuExtensions.h"
#include "MultiBoxExtender.h"
#include "LevelEditor.h"
#include "ModuleManager.h"
#include "MultiBoxBuilder.h"
#include "EditorStyle.h"
#include "Editor.h"
#include "System/Project.h"
#include "EditorModeManager.h"
#include "EditorUtilities.h"
#include "NavigationSystem.h"
#include "NavDataGenerator.h"
#include "FileManager.h"
#include "Common/GADataTable.h"
#include "Editor/EditorEngine.h"
#include "System/MMOGameMode.h"
#include "System/ResourceManager.h"
#include "PIECameraManager.h"
#include "Utilities/Utilities.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "Utilities/RecastNavMeshExporter.h"
#include "Misc/UObjectToken.h"
#include "RecastNavMesh.h"
#include "RecastNavMeshGenerator.h"
#include "RecastHelpers.h"
#include "WidgetBlueprint.h"
#include "ContextMenuExtender.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkAudioBank.h"
#include "Particles/ParticleSystem.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimComposite.h"
#include "Factories/AnimCompositeFactory.h"
#include "ContentBrowser/Public/ContentBrowserModule.h"
#include "ContentBrowser/Public/IContentBrowserSingleton.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "Animation/AnimNotifySort.h"
#include "Animation/AnimNotifyData.h"
#include "FileHelpers.h"
#include "ActionSkillEditorModule.h"
#include "ActionSkillEditor.h"
#include "ActionSkill/SkillEditorMiddleware.h"

FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelEditorMenuExtenderDelegate;
FDelegateHandle LevelEditorExtenderDelegateHandle;

#define TRANSFER_COORDINATE 0

class FEditorMenuExtensionImpl
{
private:

	static void OpenTalkEditor()
	{
		const FString TALK_EDITOR_LEVEL = "TestTalk";
		FEditorUtilities::OpenLevelByName(TALK_EDITOR_LEVEL);
	}

	static void OpenWorkshopEditor()
	{
		GLevelEditorModeTools().ActivateMode("Atelier_EM_AccessoriesEditor");
		FGlobalTabmanager::Get()->InvokeTab(AccessoriesEditorTabName);
	}

	static void OpenFurnitureEditor()
	{
		GLevelEditorModeTools().ActivateMode("Atelier_EM_AccessoriesEditor");
		FGlobalTabmanager::Get()->InvokeTab(FurnitureEditorTabName);
	}
	static void OpenMainLevel()
	{
		const FString MAIN_LEVEL = "Empty";
		FEditorUtilities::OpenLevelByName(MAIN_LEVEL);
	}

	static void OpenBattleLevel()
	{
		const FString BATTLE_LEVEL = "BattleMain";
		FEditorUtilities::OpenLevelByName(BATTLE_LEVEL);
	}

	static void SetupCameraConfigEnv()
	{
		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		FString path = PATH_ROOT_BLUEPRINTS + "Camera/BP_OfflineCameraController.BP_OfflineCameraController_C";
		AOfflineCameraController *Ctrl = FUtilities::SpawnBlueprintActor<AOfflineCameraController>(EditorWorld, path);
		//AOfflineCameraController* Ctrl = EditorWorld->SpawnActor<AOfflineCameraController>(AOfflineCameraController::StaticClass());
		Ctrl->PostInitialize();
	}

	static void OpenMapEditorLevel()
	{
		const FString MAIN_LEVEL = "MapEditor";
		FEditorUtilities::OpenLevelByName(MAIN_LEVEL);
	}

	static void OpenMapEditor()
	{
		UE_LOG(LogAtelierEditor, Warning, TEXT("Open Map Editor"));
		GLevelEditorModeTools().ActivateMode("Atelier_EM_MapEditor");
		OpenMapEditorLevel();

		FGlobalTabmanager::Get()->InvokeTab(MapEditorTabName);
	}

	static void OpenSkillEditor()
	{
		// open skill editor
		if (!FSkillEditorMiddleware::GetSkillEditor().IsValid())
		{
			IActionSkillEditorModule& ActionSKillEditorModule = FModuleManager::LoadModuleChecked<IActionSkillEditorModule>("ActionSkillEditor");
			FSkillEditorMiddleware::PostCreateEditor(ActionSKillEditorModule.CreateActionSkillEditor());
		}
	}

	static void OpenAccessoriesEditor()
	{
		UE_LOG(LogAtelierEditor, Warning, TEXT("Open Accessories Editor"));
		GLevelEditorModeTools().ActivateMode("Atelier_EM_AccessoriesEditor");

		FGlobalTabmanager::Get()->InvokeTab(AccessoriesEditorTabName);
	}

	static void OpenInteractiveEditor()
	{
		UE_LOG(LogAtelierEditor, Warning, TEXT("Open Interactive Editor"));
		GLevelEditorModeTools().ActivateMode("Atelier_EM_InteractiveEditor");

		FGlobalTabmanager::Get()->InvokeTab(InteractiveEditorTabName);
	}

	static void OpenAvatarEditor()
	{
		UE_LOG(LogAtelierEditor, Warning, TEXT("Open Avatar Editor"));
		GLevelEditorModeTools().ActivateMode("Atelier_EM_AvatarEditor");

		FGlobalTabmanager::Get()->InvokeTab(FName("AvatarEditor"));
	}

	static void AutoGenBlueprintTypeRefForLua()
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		FARFilter Filter;
		Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAnimBlueprint::StaticClass()->GetFName());
		Filter.ClassNames.Add(UWidgetBlueprint::StaticClass()->GetFName());
		Filter.ClassNames.Add(UUserDefinedStruct::StaticClass()->GetFName());

		//Filter.bRecursiveClasses = true;
		TArray< FAssetData > AssetList;
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;
		Filter.PackagePaths.Add("/Game");
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		TMap<FName, FAssetData> AssetRegistry;
		for (FAssetData Asset : AssetList )
		{
			if (!AssetRegistry.Contains(Asset.AssetName))
			{
				AssetRegistry.Add(Asset.AssetName, Asset);
			}
			else
			{
				UE_LOG(LogAtelierEditor, Error, TEXT("Duplicate Asset Name: %s"), *Asset.AssetName.ToString());
			}
		}

		auto AppendLine = [] (FString& InOut, const FString& AddStr)
		{
			InOut += AddStr + "\n";
		};

		FString OutString;
		AppendLine(OutString, "BPType = {");

		TArray<FName> NameArray;
		AssetRegistry.GetKeys(NameArray);
		NameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : NameArray)
		{
			FAssetData *Data = AssetRegistry.Find(Name);
			if (Data->AssetClass == "UserDefinedStruct" || Data->AssetClass == "ActionEventStructType")
				AppendLine(OutString, "\t" + Name.ToString() + " = \"" + Data->ObjectPath.ToString() + "\",");
			else
				AppendLine(OutString, "\t" + Name.ToString() + " = \"" + Data->ObjectPath.ToString() + "_C\",");
		}

		AppendLine(OutString, "};");
		AppendLine(OutString, "");
		//AppendLine(OutString, "return TypeRegistry");
		const FString OUT_PATH = FPaths::ProjectContentDir() + "LuaSource/GamePlay/Gen/BlueprintTypeGen.lua";
		FFileHelper::SaveStringToFile(OutString, *OUT_PATH);


		FARFilter FilterAkBank, FilterAkEvent, FilterAkBus;
		FilterAkBank.ClassNames.Add(UAkAudioBank::StaticClass()->GetFName());
		FilterAkEvent.ClassNames.Add(UAkAudioEvent::StaticClass()->GetFName());
		FilterAkBus.ClassNames.Add(UAkAuxBus::StaticClass()->GetFName());
		FilterAkBank.bRecursivePaths = true;
		FilterAkEvent.bRecursivePaths = true;
		FilterAkBus.bRecursivePaths = true;
		FilterAkBank.PackagePaths.Add("/Game");
		FilterAkEvent.PackagePaths.Add("/Game");
		FilterAkBus.PackagePaths.Add("/Game");

		//Filter.bRecursiveClasses = true;
		TArray< FAssetData > AssetListBank, AssetListEvent, AssetListBus;
		AssetRegistryModule.Get().GetAssets(FilterAkBank, AssetListBank);
		AssetRegistryModule.Get().GetAssets(FilterAkEvent, AssetListEvent);
		AssetRegistryModule.Get().GetAssets(FilterAkBus, AssetListBus);

		TMap<FName, FName> AssetRegistryBank;
		for (FAssetData Asset : AssetListBank)
		{
			if (!AssetRegistryBank.Contains(Asset.AssetName))
			{
				AssetRegistryBank.Add(Asset.AssetName, Asset.ObjectPath);
			}
			else
			{
				UE_LOG(LogAtelierEditor, Error, TEXT("Duplicate AkAudioBank Name: %s"), *Asset.AssetName.ToString());
			}
		}
		TMap<FName, FName> AssetRegistryEvent;
		for (FAssetData Asset : AssetListEvent)
		{
			if (!AssetRegistryEvent.Contains(Asset.AssetName))
			{
				AssetRegistryEvent.Add(Asset.AssetName, Asset.ObjectPath);
			}
			else
			{
				UE_LOG(LogAtelierEditor, Error, TEXT("Duplicate AkAudioEvent Name: %s"), *Asset.AssetName.ToString());
			}
		}
		TMap<FName, FName> AssetRegistryBus;
		for (FAssetData Asset : AssetListBus)
		{
			if (!AssetRegistryBus.Contains(Asset.AssetName))
			{
				AssetRegistryBus.Add(Asset.AssetName, Asset.ObjectPath);
			}
			else
			{
				UE_LOG(LogAtelierEditor, Error, TEXT("Duplicate AkAuxBus Name: %s"), *Asset.AssetName.ToString());
			}
		}

		OutString.Empty();
		AppendLine(OutString, "AkResource = {");
		AppendLine(OutString, "Bank = {");
		TArray<FName> BankNameArray;
		AssetRegistryBank.GetKeys(BankNameArray);
		BankNameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : BankNameArray)
		{
			FName Value = AssetRegistryBank.FindRef(Name);
			AppendLine(OutString, "\t" + Name.ToString() + " = \"" + Value.ToString() + "\",");
		}
		AppendLine(OutString, "},");

		AppendLine(OutString, "Event = {");
		TArray<FName> EventNameArray;
		AssetRegistryEvent.GetKeys(EventNameArray);
		EventNameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : EventNameArray)
		{
			FName Value = AssetRegistryEvent.FindRef(Name);
			AppendLine(OutString, "\t" + Name.ToString() + " = \"" + Value.ToString() + "\",");
		}
		AppendLine(OutString, "},");

		AppendLine(OutString, "Bus = {");
		TArray<FName> BusNameArray;
		AssetRegistryBus.GetKeys(BusNameArray);
		BusNameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : BusNameArray)
		{
			FName Value = AssetRegistryBus.FindRef(Name);
			AppendLine(OutString, "\t" + Name.ToString() + " = \"" + Value.ToString() + "\",");
		}
		AppendLine(OutString, "},");

		AppendLine(OutString, "};");
		AppendLine(OutString, "");
		const FString OUT_PATH2 = FPaths::ProjectContentDir() + "LuaSource/GamePlay/Gen/AkResourceGen.lua";
		FFileHelper::SaveStringToFile(OutString, *OUT_PATH2);


		//FXResourceGen Begin
		FARFilter FilterFX;
		FilterFX.ClassNames.Add(UParticleSystem::StaticClass()->GetFName());
		FilterFX.bRecursivePaths = true;
		FilterFX.PackagePaths.Add("/Game");
		TArray< FAssetData > AssetListFX;
		AssetRegistryModule.Get().GetAssets(FilterFX, AssetListFX);
		TMap<FName, FAssetData> AssetRegistryFX;
		for (FAssetData Asset : AssetListFX)
		{
			if (!AssetRegistryFX.Contains(Asset.AssetName))
			{
				AssetRegistryFX.Add(Asset.AssetName, Asset);
			}
			else
			{
				UE_LOG(LogAtelierEditor, Error, TEXT("Duplicate Asset Name: %s"), *Asset.AssetName.ToString());
			}
		}
		OutString.Empty();
		AppendLine(OutString, "FXAsset = {");
		TArray<FName> FXNameArray;
		AssetRegistryFX.GetKeys(FXNameArray);
		FXNameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : FXNameArray)
		{
			FAssetData *Data = AssetRegistryFX.Find(Name);
			AppendLine(OutString, "\t[\"" + Name.ToString() + "\"] = \"" + Data->ObjectPath.ToString() + "\",");
		}
		AppendLine(OutString, "};");
		AppendLine(OutString, "");
		const FString OUT_PATH3 = FPaths::ProjectContentDir() + "LuaSource/GamePlay/Gen/FXResourceGen.lua";
		FFileHelper::SaveStringToFile(OutString, *OUT_PATH3);
		//FxResourceGen End
	}

	/**
	 * convert *.proto file's enum to lua file
	 *
	 * @param inPBdir Directory for scanning *.proto files
	 * @param inOutLuaFileName output directory of lua's file
	 * @param inExcludePBFileNames *proto files to be excluded
	 */
	static void ConvertPBEnumToLua(const FString& inPBdir, const FString& inOutLuaFileName, const TArray<FString>& inExcludePBFileNames)
	{
		const FString tStrChatProtoDir = FPaths::ConvertRelativePathToFull(inPBdir);
		TArray<FString> tArrayPBFileNames;
		IFileManager::Get().FindFiles(tArrayPBFileNames, *tStrChatProtoDir, *FString(TEXT("*.proto")));
		TArray<FString> tArrayOutLuaLines;
		tArrayOutLuaLines.Add(FString("--This is a cmd desc file auto gen from origin-proto."));
		tArrayOutLuaLines.Add(FString("--Please do no modification."));
		tArrayOutLuaLines.Add(FString("local EMsgId = {"));
		auto tFuncIsExclude = [&](const FString& inFileName)
		{
			for (int32 tIndex = 0; tIndex < inExcludePBFileNames.Num(); ++tIndex)
			{
				if (inFileName.StartsWith(inExcludePBFileNames[tIndex]))
					return true;
			}
			return false;
		};

		for (int tFileIndex = 0; tFileIndex < tArrayPBFileNames.Num(); ++tFileIndex)
		{
			if (tFuncIsExclude(tArrayPBFileNames[tFileIndex]))
				continue;
			TArray<FString> tArrayPBFileLines;
			if (!FFileHelper::LoadFileToStringArray(tArrayPBFileLines, *(tStrChatProtoDir + tArrayPBFileNames[tFileIndex])))
				continue;
			for (int tLineIndex = 0; tLineIndex < tArrayPBFileLines.Num(); ++tLineIndex)
			{
				if (tArrayPBFileLines[tLineIndex].TrimStart().StartsWith("enum"))
				{
					// compatible with mode "enum EXXXXXXX {" and "enum EXXXXXXX"
					FString tOutLine = FString("    ") + tArrayPBFileLines[tLineIndex].TrimStart().Replace(TEXT("enum"), TEXT(""));
					tOutLine = tOutLine.Replace(TEXT("{"), TEXT(""));
					tArrayOutLuaLines.Add(tOutLine + "={");
					tLineIndex += tArrayPBFileLines[tLineIndex].TrimStart().EndsWith("{") ? 1 : 2;

					while (!tArrayPBFileLines[tLineIndex].TrimStart().StartsWith("}"))
					{
						if (tArrayPBFileLines[tLineIndex].TrimStart().StartsWith("/*"))
						{
							while (!tArrayPBFileLines[tLineIndex++].TrimStart().StartsWith("*/"));
							continue;
						}
						// enum sometimes starts with none-1 index, while in most circumstances,
						// we'll use the string-key instead of number-value as a index, so the original
						// format is kept unchanged here.
						FString cc = tArrayPBFileLines[tLineIndex++].Replace(TEXT(";"), TEXT(",")).Replace(TEXT("//"), TEXT("--"));
						tArrayOutLuaLines.Add(FString("        ") + cc);
					}
					tArrayOutLuaLines.Add("    },");
				}
			}
		}
		tArrayOutLuaLines.Add("}");
		tArrayOutLuaLines.Add("return EMsgId");
		FFileHelper::SaveStringArrayToFile(tArrayOutLuaLines, *inOutLuaFileName, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	/**
	 * generating reflection information of protocol buffers command's enum
	 *
	 * @param inPBFileName Directory for scanning *.proto files
	 * @param inOutLuaFileName output directory of lua's file
	 * @param inCmdLineStart the string command enum line start in pb file
	 * @param inLuaCmdLinePrefix additional prefix in lua's command
	 */
	static void GenPBCmdReflection(const FString& inPBFileName, const FString& inOutLuaFileName, const FString& inCmdLineStart, const FString& inLuaCmdLinePrefix)
	{
		TArray<FString> tArrayOutput;
		TArray<FString> tArrayXDAppChatCmdList;
		FFileHelper::LoadFileToStringArray(tArrayXDAppChatCmdList, *inPBFileName);
		tArrayOutput.Add(FString("--Please do no modification."));
		tArrayOutput.Add(FString("local EPBCmdInfo = {"));
		bool tFindCmd = false;
		FRegexPattern tPattern(TEXT("[a-zA-Z0-9]+"));
		FString tPackageName(TEXT("Unknow"));
		FString tPackageFlag(TEXT("// package:"));
		for (int32 tIndex = 0; tIndex < tArrayXDAppChatCmdList.Num(); ++tIndex)
		{
			if (!tFindCmd)
			{
				tFindCmd = tArrayXDAppChatCmdList[tIndex].TrimStart().StartsWith(inCmdLineStart);
				continue;
			}
			if (tArrayXDAppChatCmdList[tIndex].TrimStart().StartsWith(TEXT("}")))
				break;
			if (tArrayXDAppChatCmdList[tIndex].TrimStart().StartsWith(tPackageFlag))
			{
				tPackageName = tArrayXDAppChatCmdList[tIndex].TrimStart().RightChop(tPackageFlag.Len());
				continue;
			}
			if (tArrayXDAppChatCmdList[tIndex].TrimStart().StartsWith(TEXT("//")))
				continue;
			FString tCmd = tArrayXDAppChatCmdList[tIndex].TrimStart();
			FRegexMatcher tRegexMatcher(tPattern, tCmd);
			if (!tRegexMatcher.FindNext())
				continue;
			FString tCommandName(tRegexMatcher.GetCaptureGroup(0));
			if (!tRegexMatcher.FindNext())
				continue;
			FString tCommandValue(tRegexMatcher.GetCaptureGroup(0));
			FString tStrValueTable(TEXT("{ msgType = '") + tCommandName + TEXT("', cmdValue = ") + tCommandValue + TEXT(", package = '") + tPackageName + TEXT("' }"));
			tArrayOutput.Add(inLuaCmdLinePrefix + tCommandName + TEXT(" = ") + tStrValueTable + TEXT(","));
		}
		tArrayOutput.Add(FString("}"));
		tArrayOutput.Add(FString("return EPBCmdInfo"));
		FFileHelper::SaveStringArrayToFile(tArrayOutput, *inOutLuaFileName, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	/**
	 * generating pbc file with protocol buffers's exe, and add code register in lua files
	 * call PB's exe through .bat file traverse *.pb files and generate *.pbc file
	 *
	 * @param inBatDir path of .bat file
	 * @param iExeDir path of PB's exe file
	 */
	static void GenAndRegPBC(const FString& inBatDir, const FString& iExeDir)
	{
#if PLATFORM_WINDOWS
		// because of hard code dir in _env_regfile, don't modify tStrOutPBCDir
		const FString tStrOutPBCDir(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "LuaSource/Network/Pb/"));
		const FString tStrRegisterLuaFile = FPaths::ProjectContentDir() + "LuaSource/Network/Pbc.lua";
		// gen pb file.
		const FString tStrBatFile(TEXT("cd ") + inBatDir + iExeDir + TEXT(" ") + tStrOutPBCDir);
		_wsystem(*tStrBatFile);

		// gen pb reg.
		const FString tStrSeperator(TEXT("-- pbc reg auto-gen"));
		FString tStrLuaLines;
		FFileHelper::LoadFileToString(tStrLuaLines, *tStrRegisterLuaFile);
		int fIdx = tStrLuaLines.Find(tStrSeperator, ESearchCase::CaseSensitive, ESearchDir::FromStart);
		int lIdx = tStrLuaLines.Find(tStrSeperator, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		if (fIdx < lIdx)
		{
			tStrLuaLines.RemoveAt(fIdx, lIdx - fIdx);
			TArray<FString> tArrayFileNames;
			IFileManager::Get().FindFiles(tArrayFileNames, *tStrOutPBCDir, *FString(TEXT("*.pb")));
			FString tStrNewLines;
			tStrNewLines.Append(tStrSeperator + TEXT("\n"));
			for (int k = 0; k < tArrayFileNames.Num(); k++)
			{
				tStrNewLines.Append("    class._regFile('");
				tStrNewLines.Append(*tArrayFileNames[k]);
				tStrNewLines.Append("')\n");
			}
			tStrLuaLines.InsertAt(fIdx, tStrNewLines);
		}
		FFileHelper::SaveStringToFile(tStrLuaLines, *tStrRegisterLuaFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
#endif
	}

	/**
	 * generate lua information about xdapp's google protocol buffers
	 */
	static void AutoGenXDAppPbc()
	{
		// generating pb cmd reflection information to lua
		const FString tStrOutputFile = FPaths::ProjectContentDir() + "LuaSource/Network/EXDAppPBInfo.lua";
		const FString tStrXDAppChatProtoFile = FPaths::ProjectContentDir() + "Proto/xdapp/chat/cmd.proto";
		GenPBCmdReflection(tStrXDAppChatProtoFile, tStrOutputFile, TEXT("enum Cmd"), TEXT("    Chat"));
		// export pb's enum to lua
		const FString tStrChatPBDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "Proto/xdapp/chat/");
		const FString tStrChatPBEnumLuaFile = FPaths::ProjectContentDir() + "LuaSource/Network/EXDAppChatEnum.lua";
		const TArray<FString> tExcludeFiles;
		ConvertPBEnumToLua(tStrChatPBDir, tStrChatPBEnumLuaFile, tExcludeFiles);
		// generating and register pbc file 
		const FString tStrBatDir(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Proto/xdapp/chat/")) + TEXT(" && pb2pbcForChat.bat "));
		const FString tStrExeDir(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Proto/")));
		GenAndRegPBC(tStrBatDir, tStrExeDir);
	}


	static void AutoGenPbc()
	{
		const FString inFile = FPaths::ProjectContentDir() + "Proto/xCmd.proto";
		const FString outFile = FPaths::ProjectContentDir() + "LuaSource/Network/EProtoMap.lua";
		const FString outFile2 = FPaths::ProjectContentDir() + "LuaSource/Network/EProtoId.lua";
		TArray<FString> cmdList;
		TArray<FString> outputList;
		const FString msgPrefix = TEXT("CMD_");
		const FString msgExclude[2] = { TEXT("_START"), TEXT("_END")};
		FFileHelper::LoadFileToStringArray(cmdList, *inFile);

		// gen protoMap.
		FRegexPattern pattern(TEXT("^[a-zA-Z_]+"));
		outputList.Add(FString("--This is a cmd desc file auto gen from origin-proto."));
		outputList.Add(FString("--Please do no modification."));
		outputList.Add(FString("local EProtoMap = {"));
		for (int32 i = 0; i < cmdList.Num(); i++)
		{
			if (cmdList[i].Contains(msgPrefix))
			{
				if (!cmdList[i].Contains(msgExclude[0]) &&
					!cmdList[i].Contains(msgExclude[1]))
				{
					FRegexMatcher matcher(pattern, cmdList[i].TrimStart());
					if (matcher.FindNext())
					{
						FString temp0(matcher.GetCaptureGroup(0));
						// mac do not recoginze '\t' or '\n'
						outputList.Add(TEXT("    ") + temp0.RightChop(4) + TEXT(" = '") + temp0.Replace(TEXT("CMD_"), TEXT("Cmd.")) + TEXT("',"));
					}
				}
			}
		}

		outputList.Add(FString("}"));
		outputList.Add(FString("return EProtoMap"));
		// not sure if this file is boomed or not.
		FFileHelper::SaveStringArrayToFile(outputList, *outFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

		// gen protoId.
		outputList.Reset();
		FRegexPattern pattern2(TEXT("[a-zA-Z0-9_]+"));
		outputList.Add(FString("--This is a cmd desc file auto gen from origin-proto."));
		outputList.Add(FString("--Please do no modification."));
		outputList.Add(FString("local EProtoId = {"));
		for (int32 i = 0; i < cmdList.Num(); i++)
		{
			if (cmdList[i].Contains(msgPrefix))
			{
				if (!cmdList[i].Contains(msgExclude[0]) &&
					!cmdList[i].Contains(msgExclude[1]))
				{
					FRegexMatcher matcher(pattern2, cmdList[i].TrimStart());
					if (matcher.FindNext())
					{
						FString temp0(matcher.GetCaptureGroup(0));
						if (matcher.FindNext())
						{
							FString temp1(matcher.GetCaptureGroup(0));
							// mac do not recoginze '\t' or '\n'
							outputList.Add(TEXT("    ") + temp0.Replace(TEXT("CMD_"), TEXT("")) + TEXT(" = ") + temp1 + TEXT(","));
						}
					}
				}
			}
		}
		outputList.Add(FString("}"));
		outputList.Add(FString("return EProtoId"));
		FFileHelper::SaveStringArrayToFile(outputList, *outFile2, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

		
		// gen all msgEnum
		const FString protoDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "Proto/");
		const FString protoOutLuaFile = FPaths::ProjectContentDir() + "LuaSource/Network/EMsgId.lua";
		TArray<FString> protoFNs;
		TArray<FString> protoContent;
		TArray<FString> protoOutLua;
		IFileManager& FileManager = IFileManager::Get();
		FileManager.FindFiles(protoFNs, *protoDir, *FString(TEXT("*.proto")));
		protoOutLua.Add(FString("--This is a cmd desc file auto gen from origin-proto."));
		protoOutLua.Add(FString("--Please do no modification."));
		protoOutLua.Add(FString("local EMsgId = {"));
		for (int k = 0; k < protoFNs.Num(); k++)
		{
			if (protoFNs[k].StartsWith("xCmd"))
			{
				break;
			}
			protoContent.Reset();
			FFileHelper::LoadFileToStringArray(protoContent, *(protoDir + protoFNs[k]));
			for (int v = 0; v < protoContent.Num(); v++)
			{
				if (protoContent[v].TrimStart().StartsWith("enum"))
				{
					protoOutLua.Add(FString("    ") + protoContent[v].TrimStart().Replace(TEXT("enum"), TEXT("")) + "={");
					v += 2;
					while (!protoContent[v].TrimStart().StartsWith("}"))
					{
						if (protoContent[v].TrimStart().StartsWith("/*"))
						{
							while (!protoContent[v++].TrimStart().StartsWith("*/"));
							continue;
						}
						// enum sometimes starts with none-1 index, while in most circumstances,
						// we'll use the string-key instead of number-value as a index, so the original
						// format is kept unchanged here.
						FString cc = protoContent[v++].Replace(TEXT(";"), TEXT(",")).Replace(TEXT("//"), TEXT("--"));
						protoOutLua.Add(FString("        ") + cc);
					}
					protoOutLua.Add("    },");
				}
			}
		}
		protoOutLua.Add("}");
		protoOutLua.Add("return EMsgId");
		FFileHelper::SaveStringArrayToFile(protoOutLua, *protoOutLuaFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		
		// generating and register pbc file 
		const FString tStrBatDir(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Proto")) + TEXT(" && pb2pbc.bat "));
		const FString tStrExeDir(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Proto/")));
		GenAndRegPBC(tStrBatDir, tStrExeDir);
	}
	static void AutoGenUI()
	{

	}

	//Author: Zemin.Chen
	static void ExportAnimationData()
	{
		const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/AinmationDatas";
		FString SavePath = FEditorUtilities::OpenSaveFileDialog(DEFAULT_SAVE_PATH, TEXT("Save Ainmation Data"), TEXT("Ainmation Data File(Table_AnimationAsset.txt)|*.txt"));
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		FARFilter Filter;
		Filter.ClassNames.Add(UAnimationAsset::StaticClass()->GetFName());
		TArray< FAssetData > AssetList;
		Filter.PackagePaths.Add("/Game/Assets/_Art/_Character");
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);


		TMap<FName, TMap<FName, FString>> AssetDataMap;

		for (FAssetData Asset : AssetList)
		{
			UAnimationAsset* AnimationAsset = Cast<UAnimationAsset>(Asset.GetAsset());
			if (IsValid(AnimationAsset)) {
				FString AnimTime = FString::FromInt((int)(AnimationAsset->GetMaxCurrentTime() * 1000));//ms
				int Index;
				if (Asset.ObjectPath.ToString().FindLastChar(*TEXT("Animation"), Index))
				{
					FString UpperPath;
					UpperPath = Asset.ObjectPath.ToString().Left(Index);

					FARFilter LocalFilter;
					LocalFilter.ClassNames.Add(USkeleton::StaticClass()->GetFName());
					LocalFilter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
					LocalFilter.bRecursivePaths = true;
					LocalFilter.PackagePaths.Add(FName(*UpperPath));
					TArray< FAssetData > LocalAssetList;
					AssetRegistryModule.Get().GetAssets(LocalFilter, LocalAssetList);
					for (FAssetData LocalAsset : LocalAssetList)
					{
						if (IsValid(AnimationAsset->GetSkeleton())) {
							if (LocalAsset.AssetClass == USkeleton::StaticClass()->GetFName())
							{
								if (LocalAsset.AssetName.ToString() == AnimationAsset->GetSkeleton()->GetName())
								{

									if (!AssetDataMap.Contains(LocalAsset.AssetName))
									{
										TMap<FName, FString > LocalMap;
										LocalMap.Add(FName("name"), "\"" + LocalAsset.AssetName.ToString() + "\"");
										LocalMap.Add(Asset.AssetName, AnimTime);
										AssetDataMap.Add(LocalAsset.AssetName, LocalMap);
									}
									else
									{
										AssetDataMap[LocalAsset.AssetName].Add(Asset.AssetName, AnimTime);
									}
								}               
							}
							else if (LocalAsset.AssetClass == USkeletalMesh::StaticClass()->GetFName())
							{
								if (Cast<USkeletalMesh>(LocalAsset.GetAsset())->Skeleton == AnimationAsset->GetSkeleton())
								{
									if (!AssetDataMap.Contains(LocalAsset.AssetName))
									{
										TMap<FName, FString > LocalMap;
										LocalMap.Add(FName("name"), "\"" + LocalAsset.AssetName.ToString() + "\"");
										LocalMap.Add(FName("skeleton"), "\""+AnimationAsset->GetSkeleton()->GetName()+ "\"");
										LocalMap.Add(Asset.AssetName, AnimTime);
										AssetDataMap.Add(LocalAsset.AssetName, LocalMap);
									}
									else
									{
										AssetDataMap[LocalAsset.AssetName].Add(Asset.AssetName, AnimTime);
									}

								}
							}
						}
					}
				}
			}

		}

		FARFilter SkeletalFilter;
		SkeletalFilter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
		TArray< FAssetData > SkeletalList;
		SkeletalFilter.PackagePaths.Add("/Game/Assets/_Art/_Character");
		SkeletalFilter.bRecursivePaths = true;
		SkeletalFilter.bRecursiveClasses = true;
		AssetRegistryModule.Get().GetAssets(SkeletalFilter, SkeletalList);
		for (FAssetData Skeletal : SkeletalList)
		{
			if (!AssetDataMap.Contains(Skeletal.AssetName))
			{
				TMap<FName, FString > LocalMap;
				LocalMap.Add(FName("name"), "\"" + Skeletal.AssetName.ToString() + "\"");
				LocalMap.Add(FName("skeleton"), "\"" + Cast<USkeletalMesh>(Skeletal.GetAsset())->Skeleton->GetName() + "\"");
				AssetDataMap.Add(Skeletal.AssetName, LocalMap);
			} 
		}

		auto AppendLine = [](FString& InOut, const FString& AddStr)
		{
			InOut += AddStr + "\n";
		};

		FString OutString;
		AppendLine(OutString, "Table_AnimationAsset = {");
		int Index = 1;

		TArray<FName> NameArray;
		AssetDataMap.GetKeys(NameArray);
		NameArray.Sort([](const FName& One, const FName& Two) {
			return One < Two;
		});
		for (const FName& Name : NameArray)
		{
			AppendLine(OutString, "\t[" + FString::FromInt(Index) + "] = {");
			int LocalIndex = 1;
			TMap<FName, FString> ValueMap = AssetDataMap.FindRef(Name);
			for (TPair <FName, FString> Pair2 : ValueMap)
			{
				if (Pair2.Key == "skeleton" || Pair2.Key == "name")
				{
					AppendLine(OutString, "\t\t" + Pair2.Key.ToString() + " = " + Pair2.Value + ",");
				}
				else
				{
					AppendLine(OutString, "\t\t[" + FString::FromInt(LocalIndex) + "] = { name = \"" + Pair2.Key.ToString() + "\", time = " + Pair2.Value + " },");
					LocalIndex++;
				}
			}
			AppendLine(OutString, "\t},");
			Index++;
		}
		AppendLine(OutString, "}");
		AppendLine(OutString, "");
		FFileHelper::SaveStringToFile(OutString, *SavePath);
	}

	static void ExportNavMeshData()
	{
		const FString DEFAULT_SAVE_PATH = FPaths::ProjectContentDir() + "Data/MapNavDatas";
		FString SavePath = FEditorUtilities::OpenSaveFileDialog(DEFAULT_SAVE_PATH, TEXT("Save Nav Data"), TEXT("Nav Data File(*.obj)|*.obj"));

		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(EditorWorld);
		if (NavSys)
		{
			const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance();
			if (NavData)
			{
				if (const FNavDataGenerator* Generator = NavData->GetGenerator())
				{
					//Generator->ExportNavigationData(SavePath);
					RecastExporter::ExportNavigationData(static_cast<const FRecastNavMeshGenerator*>(Generator), SavePath);
				}
			}
		}

#if 0
		// Format Output File Name
		FString SaveDir;
		{
			int Index;
			if (SavePath.FindLastChar(*TEXT("/"), Index))
			{
				SaveDir = SavePath.Left(Index + 1);
			}
		}

		TArray<FString> Files;

		IFileManager& FileManager = IFileManager::Get();
		FileManager.FindFiles(Files, *(SavePath + "*"), true, false);
		for (int i = 0; i < Files.Num(); i++)
		{
			FString& File = Files[i];

			// Cause the file is short name, here we need combine it to full path

			File = SaveDir + File;
			UE_LOG(LogAtelierEditor, Warning, TEXT("Similar Files: %s => %s"), *File, *SavePath);
			if (File.StartsWith(SavePath))
			{
				FileManager.Move(*SavePath, *File, true);
			}
		}
#endif

#if TRANSFER_COORDINATE
		// Format Output Navmesh Data for RecastNavigation
		FString FileContent;
		FString OutputFileContent;
		FFileHelper::LoadFileToString(FileContent, *SavePath);
		if (!FileContent.IsEmpty())
		{
#if PLATFORM_WINDOWS
			TArray<FString> Lines;
			int32 LineCount = FileContent.ParseIntoArray(Lines, _T("\n"), true);
			for (int i = 0; i < LineCount; ++i)
			{
				FString& LineStr = Lines[i];
				if (!LineStr.StartsWith("v "))
				{
					OutputFileContent.Append(LineStr).Append(_T("\n"));
					continue;
				}

				TArray<FString> Vec;
				int ElementCount = LineStr.ParseIntoArray(Vec, _T(" "), true);
				if (ElementCount >= 4)
				{
					OutputFileContent.Append(Vec[0]).Append(_T(" "));
					// -X => X
					float X = FCString::Atof(*Vec[1]);
					OutputFileContent.Append(FString::SanitizeFloat(-1 * X)).Append(" ");
					// -Z => Y
					float Y = FCString::Atof(*Vec[3]);
					OutputFileContent.Append(FString::SanitizeFloat(-1 * Y)).Append(" ");
					// Y => Z
					float Z = FCString::Atof(*Vec[2]);
					OutputFileContent.Append(FString::SanitizeFloat(Z));
					OutputFileContent.Append("\n");
				}
			}
#endif
		}
		FFileHelper::SaveStringToFile(OutputFileContent, *SavePath);
#endif
	}

	static void CreateAnimPath()
	{
		UGADataTable* ActionTable					= UGADataTable::GetDataTable("ActionAnime");
		UGADataTable* NpcTable						= UGADataTable::GetDataTable("NPC");
		UGADataTable* ActionAnimeExtensionTalble	= UGADataTable::GetDataTable("ActionAnimeExtension");
		TArray<UGADataRow*> ActionRows;
		TArray<UGADataRow*> NpcRows;
		ActionTable->GetAllRows(ActionRows);
		NpcTable->GetAllRows(NpcRows);

		FString SavePath = FPaths::ProjectContentDir() + "/Table/AnimLoadInfo.csv";
		TArray<FString> OutputFileContent;
		TSet<FString>	animNameSet;
		TArray<FString> animNameArray;
		FString line = "ID";
		for (int i = 0; i < ActionRows.Num(); i++)
		{
			FString name = (ActionRows[i])->GetStr("NameEn");
			bool bIsAlreadyInSet;
			animNameSet.Add(name, &bIsAlreadyInSet);
			if (bIsAlreadyInSet == false)
			{
				line += "," + name;
				animNameArray.Add(name);
			}
		}
		OutputFileContent.Add(line);

		line = "number";
		for (int i = 0; i < animNameArray.Num(); i++)
		{
			line += ",number";
		}
		OutputFileContent.Add(line);

		const FText DefaultText = FText::AsPercent(0.0f);
		FScopedSlowTask SlowTask(100, DefaultText);
		SlowTask.MakeDialog();
		for (int i = 0; i < NpcRows.Num(); i++)
		{
			SlowTask.EnterProgressFrame(((float)i) / NpcRows.Num());
			int CfgID = (NpcRows[i])->GetNumber("ID");
			FString val = FString::FromInt(CfgID);
			line = val;
			for (int j = 0; j < animNameArray.Num(); j++)
			{
				int key = 0;
				TArray<FName> an;
				an.Push(*FString::FromInt(CfgID));
				an.Push(*animNameArray[j]);
				UGADataRow* row = ActionAnimeExtensionTalble->GetRowByArray(an);
				if (row != nullptr)
				{
					key = 4;
				}
				else
				{
					while (true)
					{
						FString resPath = NpcRows[i]->GetStr("ResourcePath");
						if (resPath.Len() > 1)
						{
							resPath = "/Game/Assets/_Art/_Character/" + resPath + "/Animation/" + animNameArray[j] + "." + animNameArray[j];
							UObject* ret = StaticLoadObject(UObject::StaticClass(), NULL, *resPath, NULL, LOAD_None, NULL);
							if (ret != nullptr)
							{
								key = 1;
								break;
							}
						}
						FString actionPath = NpcRows[i]->GetStr("ActionPath");
						if (actionPath.Len() > 1)
						{
							resPath = "/Game/Assets/_Art/_Character/" + actionPath + "/Animation/" + animNameArray[j] + "." + animNameArray[j];
							UObject* ret = StaticLoadObject(UObject::StaticClass(), NULL, *resPath, NULL, LOAD_None, NULL);
							if (ret != nullptr)
							{
								key = 2;
								break;
							}
						}
						resPath = "";
						FString modelType = NpcRows[i]->GetStr("ModelType");
						if (modelType.Len() > 1)
						{
							if (modelType == TEXT("Partner_BKH"))
							{
								resPath = "/Game/Assets/_Art/_Character/_Common/Main/Animation/BKH/" + animNameArray[j] + "." + animNameArray[j];
							}
							else if (modelType == TEXT("Partner_BKF"))
							{
								resPath = "/Game/Assets/_Art/_Character/_Common/Main/Animation/BKF/" + animNameArray[j] + "." + animNameArray[j];
							}
							UObject* ret = StaticLoadObject(UObject::StaticClass(), NULL, *resPath, NULL, LOAD_None, NULL);
							if (ret != nullptr)
							{
								key = 3;
								break;
							}
						}
						break;
					}
				}




				line += "," + FString::FromInt(key);
			}
			OutputFileContent.Add(line);
		}
		

		FFileHelper::SaveStringArrayToFile(OutputFileContent, *SavePath);
	}

	static void GenerateAnimComposite()
	{
		UE_LOG(LogAtelierEditor, Warning, TEXT("Generate Animation Composite From AnimSequence Begin"));

		//if (CurrentSelectedPaths.Num() == 0)
		//	return;

		//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		//FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();

		//const TArray<FAnimNotifySortItem>& NotifySortItems = FAnimNotifySortConfig::GetInstance()->GetAllSortItem();

		//auto ReflashFloderFunc = [&](const FName& Path)
		//{
		//	FARFilter Filter;
		//	Filter.PackagePaths.Add(Path);
		//	Filter.ClassNames.Emplace(TEXT("ObjectRedirector"));

		//	// Query for a list of assets in the selected paths
		//	TArray<FAssetData> AssetList;
		//	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//	if (AssetList.Num() > 0)
		//	{
		//		TArray<UObject*> Objects;
		//		//TArray<FString> ObjectPaths;
		//		for (const auto& Asset : AssetList)
		//		{
		//			//ObjectPaths.Add(Asset.ObjectPath.ToString());
		//			Objects.Add(CastChecked<UObjectRedirector>(Asset.GetAsset()));
		//		}
		//		TArray<UObjectRedirector*> Redirectors;
		//		for (auto Object : Objects)
		//		{
		//			auto Redirector = CastChecked<UObjectRedirector>(Object);
		//			Redirectors.Add(Redirector);
		//		}
		//		AssetToolsModule.Get().FixupReferencers(Redirectors);
		//	}
		//};

		////记录当前AnimComposite和对应Track数据，转换完成后刷新Track
		//TMap<FName, TArray<FName>> FreshTrackAnimCompositeMap;
		//{
		//	FARFilter Filter;
		//	Filter.ClassNames.Add(UAnimComposite::StaticClass()->GetFName());
		//	for (auto Path : CurrentSelectedPaths)
		//	{
		//		Filter.PackagePaths.Add(FName("/Game/Assets/_Art"));
		//	}
		//	Filter.bRecursivePaths = true;

		//	TArray< FAssetData > AssetList;
		//	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//	for (FAssetData AssetData : AssetList)
		//	{
		//		UAnimComposite* AnimComposite = Cast<UAnimComposite>(AssetData.GetAsset());
		//		FAnimTrack& Track = AnimComposite->AnimationTrack;
		//		TArray<FAnimSegment>& Segments = Track.AnimSegments;
		//		if (Segments.Num() > 0)
		//		{
		//			TArray<FName> Names;
		//			for (auto iter = Segments.CreateIterator(); iter; iter++)
		//			{
		//				FAnimSegment& Seg = *iter;
		//				Names.Add(Seg.AnimReference->GetOutermost()->FileName);
		//			}
		//			FreshTrackAnimCompositeMap.Add(AssetData.PackageName, Names);
		//		}
		//	}
		//}

		////记录当前AnimMontage和对应Track数据，转换完成后刷新Track
		//TMap<FName, TMap<FName, TArray<FName>>> FreshTrackAnimMontageMap;
		//{
		//	FARFilter Filter;
		//	Filter.ClassNames.Add(UAnimMontage::StaticClass()->GetFName());
		//	for (auto Path : CurrentSelectedPaths)
		//	{
		//		Filter.PackagePaths.Add(FName("/Game/Assets/_Art"));
		//	}
		//	Filter.bRecursivePaths = true;

		//	TArray< FAssetData > AssetList;
		//	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//	for (FAssetData AssetData : AssetList)
		//	{
		//		UAnimMontage* AnimMontage = Cast<UAnimMontage>(AssetData.GetAsset());
		//		if (AnimMontage->SlotAnimTracks.Num() > 0)
		//		{
		//			TMap<FName, TArray<FName>> TempMap;
		//			for (auto iter = AnimMontage->SlotAnimTracks.CreateIterator(); iter; iter++)
		//			{
		//				FSlotAnimationTrack& SlotAnimationTrack = *iter;
		//				FAnimTrack& Track = SlotAnimationTrack.AnimTrack;
		//				TArray<FAnimSegment>& Segments = Track.AnimSegments;
		//				if (Segments.Num() > 0)
		//				{
		//					TArray<FName> Names;
		//					for (auto iter = Segments.CreateIterator(); iter; iter++)
		//					{
		//						FAnimSegment& Seg = *iter;
		//						Names.Add(Seg.AnimReference->GetOutermost()->FileName);
		//					}
		//					TempMap.Add(SlotAnimationTrack.SlotName, Names);
		//				}
		//			}
		//			if (TempMap.Num() > 0)
		//			{
		//				FreshTrackAnimMontageMap.Add(AssetData.PackageName, TempMap);
		//			}
		//		}
		//	}
		//}

		//TMap<FName, bool> AllPathMap;

		////生成目录信息
		//{
		//	FARFilter Filter;
		//	Filter.bRecursivePaths = true;
		//	Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());

		//	for (auto Path : CurrentSelectedPaths)
		//	{
		//		Filter.PackagePaths.Add(FName(*Path));
		//	}

		//	TArray< FAssetData > AssetList;
		//	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//	for (FAssetData AssetData : AssetList)
		//	{
		//		AllPathMap.Add(AssetData.PackagePath, true);
		//	}
		//}

		//auto SavePathFunc = [&](const FName& Path)
		//{
		//	FARFilter Filter;
		//	Filter.PackagePaths.Add(Path);

		//	TArray< FAssetData > AssetList;
		//	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//	TArray<UPackage*> PackagesToSave;
		//	for (FAssetData AssetData : AssetList)
		//	{
		//		PackagesToSave.Add(AssetData.GetAsset()->GetOutermost());
		//	}

		//	TArray<UPackage*> FailedPackages;
		//	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false, &FailedPackages);
		//	if (FailedPackages.Num() > 0)
		//	{
		//		for (auto iter = FailedPackages.CreateIterator(); iter; ++iter)
		//		{
		//			UPackage* Pkg = *iter;
		//			UE_LOG(LogAtelierEditor, Warning, TEXT("Save Faileds: %s"), *(Pkg->FileName.ToString()));
		//		}
		//	}
		//};

		//for (auto iter = AllPathMap.CreateIterator(); iter; ++iter)
		//{
		//	FName CurrentPath = iter->Key;

		//	//检查目录是否处理过
		//	bool Handled = false;
		//	{
		//		FARFilter Filter;
		//		Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
		//		Filter.PackagePaths.Add(CurrentPath);

		//		TArray< FAssetData > AssetList;
		//		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//		//TArray<FName> AnimNotifyDataPackageNames;

		//		for (FAssetData AssetData : AssetList)
		//		{
		//			if (AssetData.PackageName.ToString().Right(2).Equals("__"))
		//			{
		//				Handled = true;
		//				break;
		//			}
		//		}
		//	}
		//	if (Handled)
		//		continue;

		//	UE_LOG(LogAtelierEditor, Warning, TEXT("Hanlde Path : %s"), *(CurrentPath.ToString()));

		//	TArray<FName> NewAnimSequencePackageNames;
		//	TArray<FName> NewAnimCompositePackageNames;
		//	TMap<FName, UAnimSequenceBase*> FinalAnimCompositePackageNameObjectMap;

		//	TMap<FName, FName> NameCacheMap;

		//	//重命名AnimSequence
		//	{
		//		FARFilter Filter;
		//		Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
		//		Filter.PackagePaths.Add(CurrentPath);

		//		TArray< FAssetData > AssetList;
		//		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//		//TArray<FName> AnimNotifyDataPackageNames;

		//		for (FAssetData AssetData : AssetList)
		//		{
		//			FString AnimSequencePath = AssetData.PackagePath.ToString();
		//			FString AnimSequenceName = AssetData.AssetName.ToString();
		//			FString AnimSequencePackageName = AssetData.PackageName.ToString();
		//			FString PreFix = "__";

		//			FString NewAnimSequenceName;
		//			NewAnimSequenceName.Append(PreFix).Append(AnimSequenceName);
		//			UAnimSequence* AnimSequenceObject = CastChecked<UAnimSequence>(AssetData.GetAsset());

		//			TArray<FAssetRenameData> AssetsAndNames;
		//			FAssetRenameData RenameData(MakeWeakObjectPtr<UObject>(AnimSequenceObject), AnimSequencePath, NewAnimSequenceName);
		//			AssetsAndNames.Add(RenameData);
		//			AssetToolsModule.Get().RenameAssets(AssetsAndNames);
		//			FAssetRegistryModule::AssetRenamed(AnimSequenceObject, AnimSequencePath);
		//			AnimSequenceObject->MarkPackageDirty();

		//			FString NewAnimSequencePackageName;
		//			NewAnimSequencePackageName.Append(AnimSequencePath).Append("/").Append(NewAnimSequenceName);
		//			NewAnimSequencePackageNames.Add(FName(*NewAnimSequencePackageName));

		//			UE_LOG(LogAtelierEditor, Warning, TEXT("Rename AnimSequence : [%s][%s]"), *(AnimSequencePackageName), *(NewAnimSequencePackageName));

		//			NameCacheMap.Add(FName(*NewAnimSequencePackageName), FName(*AnimSequencePackageName));
		//		}
		//	}

		//	//刷新目录
		//	ReflashFloderFunc(CurrentPath);

		//	SavePathFunc("/Game/Assets/_Art");

		//	//生成AnimComposite
		//	{
		//		FARFilter Filter;
		//		Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
		//		for (auto iter = NameCacheMap.CreateIterator(); iter; ++iter)
		//		{
		//			Filter.PackageNames.Add(iter->Key);
		//		}
		//		//Filter.PackageNames = NewAnimSequencePackageNames;

		//		TArray< FAssetData > AssetList;
		//		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//		for (FAssetData AssetData : AssetList)
		//		{
		//			UAnimSequence* AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());
		//			UAnimCompositeFactory* CompositeFactory = NewObject<UAnimCompositeFactory>();
		//			CompositeFactory->SourceAnimation = AnimSequence;

		//			FString AnimCompositeName = AssetData.AssetName.ToString();//AssetData.AssetName.ToString();
		//			AnimCompositeName = AnimCompositeName.Right(AnimCompositeName.Len() - 2);
		//			//AnimCompositeName.Append("Composite");

		//			FString AnimCompositePackageName = NameCacheMap.Find(AssetData.PackageName)->ToString(); //AssetData.PackageName.ToString();
		//			//AnimCompositePackageName.Append("Composite");
		//			NewAnimCompositePackageNames.Add(FName(*AnimCompositePackageName));

		//			UAnimationAsset* NewAsset = Cast<UAnimationAsset>(AssetToolsModule.Get().CreateAsset(AnimCompositeName, FPackageName::GetLongPackagePath(AnimCompositePackageName), UAnimComposite::StaticClass(), CompositeFactory));

		//			if (NewAsset)
		//			{
		//				NewAsset->MarkPackageDirty();
		//			}
		//			else
		//			{
		//				UE_LOG(LogAtelierEditor, Warning, TEXT("Create AnimComposite Failed : [%s]"), *(AnimCompositePackageName));
		//			}
		//		}
		//	}

		//	//刷新目录
		//	ReflashFloderFunc(CurrentPath);

		//	//保存
		//	SavePathFunc(CurrentPath);

		//	//迁移NotifyData
		//	{
		//		FARFilter Filter;

		//		//Filter.PackageNames = ToDeleteAnimNotifyDataPackageNames;
		//		TArray< FAssetData > AssetList;
		//		//AssetRegistryModule.Get().GetAssets(Filter, AssetList);
		//		//ObjectTools::DeleteAssets(AssetList);
		//		//ReflashFloderFunc();

		//		Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
		//		//Filter.PackageNames = NewAnimSequencePackageNames;
		//		for (auto iter = NameCacheMap.CreateIterator(); iter; ++iter)
		//		{
		//			Filter.PackageNames.Add(iter->Key);
		//		}
		//		AssetList.Empty();
		//		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//		for (FAssetData AssetData : AssetList)
		//		{
		//			UAnimSequence* AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());

		//			//FString AnimSequencePackageName = AssetData.PackageName.ToString();
		//			FString OriginalPackageName = NameCacheMap.Find(AssetData.PackageName)->ToString();//AnimSequencePackageName.Left(AnimSequencePackageName.Len() - 2);

		//			UPackage* Pkg = LoadPackage(nullptr, *OriginalPackageName, 0);
		//			UAnimComposite* AnimComposite = LoadObject<UAnimComposite>(Pkg, *Pkg->GetPathName());
		//			if (!AnimComposite)
		//			{
		//				UE_LOG(LogAtelierEditor, Warning, TEXT("Load AnimComposite Failed : [%s]"), *(OriginalPackageName));
		//				continue;
		//			}
		//			AnimComposite->DeepCopyNotifies(AnimSequence);
		//			AnimComposite->MarkPackageDirty();

		//			AnimSequence->RemoveAllNotifies();
		//			AnimSequence->MarkPackageDirty();
		//		}
		//	}

		//	//刷新目录
		//	ReflashFloderFunc(CurrentPath);

		//	//保存
		//	SavePathFunc(CurrentPath);
		//}

		////刷新老的AnimComposite
		//{
		//	TArray<UPackage*> PackagesToSave;
		//	for (auto iter = FreshTrackAnimCompositeMap.CreateIterator(); iter; ++iter)
		//	{
		//		FName Key = iter->Key;
		//		TArray<FName> Values = iter->Value;
		//		UPackage* Pkg = LoadPackage(nullptr, *(Key.ToString()), 0);
		//		UAnimComposite* AnimComposite = LoadObject<UAnimComposite>(Pkg, *Pkg->GetPathName());
		//		FAnimTrack& Track = AnimComposite->AnimationTrack;
		//		TArray<FAnimSegment>& Segments = Track.AnimSegments;
		//		for (int i = 0; i < Segments.Num(); ++i)
		//		{
		//			UPackage* Pkg = LoadPackage(nullptr, *(Values[i].ToString()), 0);
		//			UAnimComposite* AnimComposite = LoadObject<UAnimComposite>(Pkg, *Pkg->GetPathName());
		//			FAnimSegment& Seg = Segments[i];
		//			Seg.AnimReference = AnimComposite;
		//		}
		//		PackagesToSave.Add(Pkg);
		//	}

		//	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
		//}

		////刷新老的AnimMontage
		//{
		//	//TMap<FName, TMap<FName, TArray<FName>>> FreshTrackAnimMontageMap;
		//	TArray<UPackage*> PackagesToSave;

		//	for (auto iter = FreshTrackAnimMontageMap.CreateIterator(); iter; ++iter)
		//	{
		//		FName Key = iter->Key;
		//		TMap<FName, TArray<FName>>& SlotAnimMap = iter->Value;
		//		UPackage* Pkg = LoadPackage(nullptr, *(Key.ToString()), 0);
		//		UAnimMontage* AnimMontage = LoadObject<UAnimMontage>(Pkg, *Pkg->GetPathName());
		//		for (auto iter = AnimMontage->SlotAnimTracks.CreateIterator(); iter; ++iter)
		//		{
		//			FSlotAnimationTrack& SlotAnimationTrack = *iter;
		//			FAnimTrack& Track = SlotAnimationTrack.AnimTrack;
		//			TArray<FAnimSegment>& Segments = Track.AnimSegments;
		//			TArray<FName>& Array = *(SlotAnimMap.Find(SlotAnimationTrack.SlotName));
		//			for (int i = 0; i < Segments.Num(); ++i)
		//			{
		//				UPackage* Pkg = LoadPackage(nullptr, *(Array[i].ToString()), 0);
		//				UAnimComposite* AnimComposite = LoadObject<UAnimComposite>(Pkg, *Pkg->GetPathName());
		//				FAnimSegment& Seg = Segments[i];
		//				Seg.AnimReference = AnimComposite;
		//			}
		//		}
		//		PackagesToSave.Add(Pkg);
		//	}

		//	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
		//}

		UE_LOG(LogAtelierEditor, Warning, TEXT("Generate Animation Composite From AnimSequence End"));

		//CurrentSelectedPaths.Empty();
	}

	static void CheckParticle()
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		FARFilter Filter;
		Filter.ClassNames.Add(UParticleSystem::StaticClass()->GetFName());

		//Filter.bRecursiveClasses = true;
		TArray< FAssetData > AssetList;
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);
		Filter.bRecursiveClasses = true;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add("/Game");
		int ErrorNum = 0;

		static const FName NAME_AssetCheck("AssetCheck");
		FMessageLog AssetCheckLog(NAME_AssetCheck);

		for (FAssetData Asset : AssetList)
		{
			UParticleSystem *Ps = Cast<UParticleSystem>(Asset.GetAsset());
			if (Ps && Ps->bUseFixedRelativeBoundingBox == false)
			{
				++ErrorNum;
				const FText Message = FText::Format(
					NSLOCTEXT("CheckParticle", "ParticleSystem_ShouldSetFixedRelativeBoundingBox", "Particle system {0} Should Use FixedRelativeBoundingBox."),
					FText::AsCultureInvariant(Ps->GetPathName()));
				AssetCheckLog.Error()
					->AddToken(FUObjectToken::Create(Ps))
					->AddToken(FTextToken::Create(Message));
			}
		}
		if(ErrorNum > 0)
			AssetCheckLog.Open(EMessageSeverity::Error);
	}

	//检查材质资源的合规性：
	static void CheckMaterial() 
	{
		CheckRootMaterailInstance();
		CheckDefaultMaterialHasAdditional();
	}

	//检查角色文件夹下SkeletalMesh的材质的根Instance是否都在Default文件夹下
	//检查角色文件夹下SkeletalMesh的材质中的静态switch参数自材质是否和Root材质保持一致
	static void CheckRootMaterailInstance()
	{
		//筛选角色资源
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray< FAssetData > AssetList;
		FARFilter Filter;
		Filter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add("/Game/Assets/_Art/_Character");
		AssetRegistryModule.Get().GetAssets(Filter, AssetList);

		//注册Log信息
		static const FName NAME_AssetCheck("AssetCheck");
		FMessageLog AssetCheckLog(NAME_AssetCheck);

		FString DefaultPath = "/Game/Assets/_Art/_Shader/_Base/_Character/Default";
		int ErrorNum = 0;
		for (FAssetData Asset : AssetList)
		{
			USkeletalMesh *Sm = Cast<USkeletalMesh>(Asset.GetAsset());
			TArray<FSkeletalMaterial> materials = Sm->Materials;
			for (int i = 0; i < materials.Num(); i++)
			{
				UMaterialInterface *baseParent = materials[i].MaterialInterface;

				while (true)
				{
					UMaterialInstance* toInstance = Cast<UMaterialInstance>(baseParent);
					if (toInstance && toInstance->Parent)
					{
						UMaterialInstance* parentToInstance = Cast<UMaterialInstance>(toInstance->Parent);
						if (parentToInstance)
							baseParent = parentToInstance;
						else
							break;
					}
					else
					{
						break;
					}
				}
				if (baseParent != nullptr)
				{
					FString OriginPathName = UKismetSystemLibrary::GetPathName(baseParent);
					if (OriginPathName.Len() > DefaultPath.Len() && OriginPathName.Left(DefaultPath.Len()) != DefaultPath)
					{
						++ErrorNum;
						AssetCheckLog.Error()
							->AddToken(FUObjectToken::Create(Sm))
							->AddToken(FTextToken::Create(FText::FromString("Root Material Instance: ")))
							->AddToken(FUObjectToken::Create(baseParent))
							->AddToken(FTextToken::Create(FText::FromString(" doesn't in the default folder")));
					}

					TArray<FMaterialParameterInfo> MineOutParameterInfo;
					TArray<FGuid> MineOutParameterIds;
					UMaterialInstance* mineInstance = Cast<UMaterialInstance>(materials[i].MaterialInterface);
					if (mineInstance != nullptr)
					{
						mineInstance->GetAllStaticSwitchParameterInfo(MineOutParameterInfo, MineOutParameterIds);

						TArray<FMaterialParameterInfo> ParentOutParameterInfo;
						TArray<FGuid> ParentOutParameterIds;
						baseParent->GetAllStaticSwitchParameterInfo(ParentOutParameterInfo, ParentOutParameterIds);
						if (MineOutParameterInfo == ParentOutParameterInfo)
						{
							for (FMaterialParameterInfo Info : MineOutParameterInfo)
							{
								bool minevalue;
								FGuid mineid;
								mineInstance->GetStaticSwitchParameterValue(Info, minevalue, mineid);

								bool basevalue;
								FGuid baseid;
								baseParent->GetStaticSwitchParameterValue(Info, basevalue, baseid);

								if (minevalue != basevalue)
								{
									++ErrorNum;
									AssetCheckLog.Error()
										->AddToken(FUObjectToken::Create(mineInstance))
										->AddToken(FTextToken::Create(FText::FromString(" has differenc switch parameter with root:" + Info.ToString())))
										->AddToken(FUObjectToken::Create(baseParent));
								}
							}
						}
						else
						{
							++ErrorNum;
							AssetCheckLog.Error()
								->AddToken(FUObjectToken::Create(Sm))
								->AddToken(FTextToken::Create(FText::FromString(" has differenc switch parameter with root")));
						}
					}
				}
			}
		}
		if (ErrorNum > 0)
			AssetCheckLog.Open(EMessageSeverity::Error);
	}

	//所有Default文件夹下的材质在Aditional文件夹下是否有对应的替换材质
	static void CheckDefaultMaterialHasAdditional()
	{
		//筛选角色资源
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		
		TArray< FAssetData > DefaultAssetList;
		FARFilter DefaultFilter;
		DefaultFilter.ClassNames.Add(UMaterialInstanceConstant::StaticClass()->GetFName());
		DefaultFilter.PackagePaths.Add("/Game/Assets/_Art/_Shader/_Base/_Character/Default");
		AssetRegistryModule.Get().GetAssets(DefaultFilter, DefaultAssetList);

		TArray< FAssetData > AdditionalAssetList;
		FARFilter AdditionalFilter;
		AdditionalFilter.ClassNames.Add(UMaterialInstanceConstant::StaticClass()->GetFName());
		AdditionalFilter.PackagePaths.Add("/Game/Assets/_Art/_Shader/_Base/_Character/Additional");
		AssetRegistryModule.Get().GetAssets(AdditionalFilter, AdditionalAssetList);

		TArray<FString> AdditionalAssetNameList;
		TMap<FString, FAssetData> AdditionalAssetDic;
		for (FAssetData AdditionalAsset : AdditionalAssetList)
		{
			AdditionalAssetNameList.Add(AdditionalAsset.AssetName.ToString());
			AdditionalAssetDic.Add(AdditionalAsset.AssetName.ToString(), AdditionalAsset);
		}

		//注册Log信息
		static const FName NAME_AssetCheck("AssetCheck");
		FMessageLog AssetCheckLog(NAME_AssetCheck);

		int ErrorNum = 0;
		for (FAssetData DefaultAsset : DefaultAssetList)
		{
			UMaterialInstanceConstant *DefaultMic = Cast<UMaterialInstanceConstant>(DefaultAsset.GetAsset());
			TArray<FString> NameList;
			TArray<FString> ConfigList;
			FString DefaultName = DefaultMic->GetFName().ToString();
			NameList.Add(DefaultName + "_Hit");
			NameList.Add(DefaultName + "_Avatar");
			NameList.Add(DefaultName + "_Death");
			NameList.Add(DefaultName + "_Rim");
			ConfigList.Add("Hit");
			ConfigList.Add("Avatar");
			ConfigList.Add("Death");
			ConfigList.Add("Rim");

			for (int i = 0; i < 4; i++) 
			{
				FString name = NameList[i];
				FString config = ConfigList[i];

				if (!AdditionalAssetNameList.Contains(name))
				{
					++ErrorNum;
					AssetCheckLog.Error()
						->AddToken(FUObjectToken::Create(DefaultMic))
						->AddToken(FTextToken::Create(FText::FromString("don't have [" + config + "] material in additional folder")));
				}
				else
				{
					FAssetData AdditionalAsset = AdditionalAssetDic.FindRef(name);
					UMaterialInstanceConstant *AdditionalMic = Cast<UMaterialInstanceConstant>(AdditionalAsset.GetAsset());
					if (!CompareDefaultAndAdditional(DefaultMic, AdditionalMic, config))
					{
						++ErrorNum;
						AssetCheckLog.Error()
							->AddToken(FUObjectToken::Create(AdditionalMic))
							->AddToken(FTextToken::Create(FText::FromString(" don't have correct static switch parameter by")))
							->AddToken(FUObjectToken::Create(DefaultMic));
					}
				}
			}
		}
		if (ErrorNum > 0)
			AssetCheckLog.Open(EMessageSeverity::Error);
	}

	static bool CompareDefaultAndAdditional(UMaterialInstanceConstant *DefaultMic, UMaterialInstanceConstant *AdditionalMic, FString config)
	{
		bool result = false;

		TArray<FMaterialParameterInfo> DefaultOutParameterInfo;
		TArray<FGuid> DefaultOutParameterIds;
		DefaultMic->GetAllStaticSwitchParameterInfo(DefaultOutParameterInfo, DefaultOutParameterIds);

		TArray<FMaterialParameterInfo> AdditionalOutParameterInfo;
		TArray<FGuid> AdditionalOutParameterIds;
		AdditionalMic->GetAllStaticSwitchParameterInfo(AdditionalOutParameterInfo, AdditionalOutParameterIds);
		
		if (DefaultOutParameterInfo == AdditionalOutParameterInfo)
		{
			for (FMaterialParameterInfo Info : DefaultOutParameterInfo)
			{
				bool defaultvalue;
				FGuid defaultid;
				DefaultMic->GetStaticSwitchParameterValue(Info, defaultvalue, defaultid);

				bool additionalvalue;
				FGuid additionalid;
				AdditionalMic->GetStaticSwitchParameterValue(Info, additionalvalue, additionalid);

				if (defaultvalue != additionalvalue)
				{
					if (config == "Avatar") 
					{
						if (Info.Name != "LightFolloCamera")
						{
							result = false;
							break;
						}
					}
					else if (config == "Death")
					{
						if (Info.Name != "UseDeathDissolve")
						{
							result = false;
							break;
						}
					}
					else if (config == "Hit")
					{
						if (Info.Name != "UseAdditionalRim")
						{
							result = false;
							break;
						}
					}
					else if (config == "Rim") 
					{
						if (Info.Name != "UseAdditionalRim")
						{
							result = false;
							break;
						}
					}
				}
			}
		}
		else
		{
			result = false;
		}

		return result;
	}

	public:
		static TArray<FString> CurrentSelectedPaths;

public:
	static TSharedRef< SWidget > GenerateOpenBlueprintMenuContent()
	{
		const bool bShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);
		// ~Begin: Open Main Level
		{
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Main Level"),
				FText::FromString("Open Main Level"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenMainLevel)),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Battle Level"),
				FText::FromString("Open Battle Level"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenBattleLevel)),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				FText::FromString("Setup Camera Config Env"),
				FText::FromString("Setup Camera Config Env"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::SetupCameraConfigEnv)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}
		// ~End: Open Main Level

		// ~Begin: Talk Editor
		{
			MenuBuilder.BeginSection("TalkEditor", FText::FromString("TalkEditor"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Talk Editor"),
				FText::FromString("Open Talk Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenTalkEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
		// ~End: Talk Editor

		// ~Begin: Workshop Editor
		{
			MenuBuilder.BeginSection("WorkshopEditor", FText::FromString("WorkshopEditor"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Workshop Editor"),
				FText::FromString("Open Workshop Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenWorkshopEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Furniture Editor"),
				FText::FromString("Open Furniture Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenFurnitureEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
		// ~End: Workshop Editor

		// ~Begin: Map Editor
		// MapEditor Entrance
		MenuBuilder.BeginSection("MapEditor", FText::FromString("MapEditor"));
		{
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Map Editor"),
				FText::FromString("Open Map Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenMapEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Skill Editor"),
				FText::FromString("Open Skill Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenSkillEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				FText::FromString("Export Nav Mesh Data"),
				FText::FromString("Export Nav Mesh Data"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::ExportNavMeshData)),
				NAME_None,
				EUserInterfaceActionType::Button
			);

			MenuBuilder.AddMenuEntry(
				FText::FromString("Export Animation Data"),
				FText::FromString("Export Animation Data"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::ExportAnimationData)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}
		MenuBuilder.EndSection();

		// ~End: Map Editor

		// ~Begin: Sockets Editor
		{
			MenuBuilder.BeginSection("CustomSocketsEditor", FText::FromString("CustomSocketsEditor"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Accessories Editor"),
				FText::FromString("Open Accessories Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenAccessoriesEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Interactive Editor"),
				FText::FromString("Open Interactive Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenInteractiveEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Open Avatar Editor"),
				FText::FromString("Open Avatar Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OpenAvatarEditor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
		// ~End: Sockets Editor



		// ~Begin: Lua Editor
		{
			MenuBuilder.BeginSection("Lua Tools", FText::FromString("Lua Tools"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Gen Blueprint Type"),
				FText::FromString("Gen Blueprint Type"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::AutoGenBlueprintTypeRefForLua)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Gen Pbc"),
				FText::FromString("Gen Pbc"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::AutoGenPbc)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Gen XDApp Pbc"),
				FText::FromString("Gen XDApp Pbc"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::AutoGenXDAppPbc)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Gen lua UI"),
				FText::FromString("Gen lua UI"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::AutoGenUI)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
		// ~End: Lua Editor

		// ~Begin: Anim Tools
		{
			MenuBuilder.BeginSection("Anim Tools", FText::FromString("Anim Tools"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Gen AnimFile Path"),
				FText::FromString("Gen AnimFile Path"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::CreateAnimPath)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			//MenuBuilder.AddMenuEntry(
			//	FText::FromString("Gen AnimComposite From AnimSequence"),
			//	FText::FromString("Gen AnimComposite From AnimSequence. The new AnimComposite File will use the AnimSequence's filename and the original AnimSequence's filename will add postfix '__'"),
			//	FSlateIcon(),
			//	FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::GenerateAnimComposite)),
			//	NAME_None,
			//	EUserInterfaceActionType::Button
			//);
			MenuBuilder.EndSection();
		}
		// ~End: Anim Tools

		{
			MenuBuilder.BeginSection("Check Resource Tools", FText::FromString("Check Resource Tools"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Check Particle"),
				FText::FromString("Check Particle"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::CheckParticle)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.AddMenuEntry(
				FText::FromString("Check Material"),
				FText::FromString("Check Material"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::CheckMaterial)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
		return MenuBuilder.MakeWidget();
	}

	static void CreateToolbarEntries(FToolBarBuilder& ToolbarBuilder)
	{
		ToolbarBuilder.BeginSection("Atelier");

		ToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateStatic(&FEditorMenuExtensionImpl::GenerateOpenBlueprintMenuContent),
			FText::FromString("Atelier Tools"),
			FText::FromString("List Of Atelier Tools"),
			//FSlateIcon()
			//FSlateIcon(FEditorStyle::GetStyleSetName(), "ConfigEditor.TabIcon")
			FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.OpenLevelBlueprint")
		);
		ToolbarBuilder.EndSection();
	}

	static void OnMenuExecute()
	{}

	static void CreateMenuEntries(FMenuBuilder& MenuBuilder)
	{
		FUIAction Action(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::OnMenuExecute));

		MenuBuilder.BeginSection(TEXT("Atelier"), FText::FromString("Test"));

		MenuBuilder.AddMenuEntry(
			FText::FromString("Menu Test2"),
			FText::FromString("Menu Test Tips"),
			FSlateIcon(),
			Action,
			NAME_None,
			EUserInterfaceActionType::Button);

		MenuBuilder.EndSection();
	}

	static TSharedRef<FExtender> CreateContentBrowserMenuEntries(const TArray<FString>& Path)
	{
		FEditorMenuExtensionImpl::CurrentSelectedPaths = Path;
		// Create extender that contains a delegate that will be called to get information about new context menu items
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		// Create a Shared-pointer delegate that keeps a weak reference to object
		// "NewFolder" is a hook name that is used by extender to identify externders that will extend path context menu
		MenuExtender->AddMenuExtension("NewFolder", EExtensionHook::After, TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateStatic(&FEditorMenuExtensionImpl::_CreateContentBrowserMenuEntries));
		return MenuExtender.ToSharedRef();
	}

	static void _CreateContentBrowserMenuEntries(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.BeginSection(TEXT("Atelier"), FText::FromString("Atelier"));

		MenuBuilder.AddMenuEntry(
			FText::FromString("Convert AnimSequences to AnimComposites"),
			FText::FromString("Convert AnimSequences to AnimComposites"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FEditorMenuExtensionImpl::GenerateAnimComposite)),
			NAME_None,
			EUserInterfaceActionType::Button
		);

		MenuBuilder.EndSection();
	}
};

TArray<FString> FEditorMenuExtensionImpl::CurrentSelectedPaths = TArray<FString>();

void FEditorMenuExtensions::ExtendMenus()
{
	if (!GIsEditor)
		return;
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddMenuExtension(
		TEXT("EditMain"),
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(&FEditorMenuExtensionImpl::CreateMenuEntries)
	);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);
}

void FEditorMenuExtensions::ExtendToolbars()
{
	if (!GIsEditor)
		return;
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddToolBarExtension(
		TEXT("Game"),
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateStatic(&FEditorMenuExtensionImpl::CreateToolbarEntries)
	);

	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(Extender);
}

void FEditorMenuExtensions::ExtendContexMenu()
{
	if (!GIsEditor)
		return;
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	LevelEditorMenuExtenderDelegate = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&ContextMenuExtender::ExtendLevelViewportContextMenu);
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	MenuExtenders.Add(LevelEditorMenuExtenderDelegate);
	LevelEditorExtenderDelegateHandle = MenuExtenders.Last().GetHandle();
}

void FEditorMenuExtensions::ExtendContentBrowserContextMenu()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& MenuExtenderDelegates = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	// Create new delegate that will be called to provide our menu extener
	MenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&FEditorMenuExtensionImpl::CreateContentBrowserMenuEntries));
}
