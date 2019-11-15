#pragma once
#include "LevelEditor.h"
#include "ModuleManager.h"
#include "Application/SlateApplication.h"
#include "ILevelViewport.h"
#include "Camera/TransitiveCameraActor.h"
#include "MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GABlueprintFunctionLibrary.h"

struct ContextMenuExtender
{
	static void TransitToCamera(UTransitiveCameraActorComponent* TransitComp)
	{
		if (TransitComp)
		{
			TransitComp->PrevTransit();
		}
	}

	static void AddTransitToCameraMenu(FMenuBuilder& MenuBuilder, UTransitiveCameraActorComponent* TransitComp)
	{
		// ~Begin: TransitiveCameraActor
		{
			MenuBuilder.BeginSection("TransitiveCameraActor", FText::FromString("TransitiveCameraActor"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Transit To Camera"),
				FText::FromString("Transit To Camera"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ContextMenuExtender::TransitToCamera, TransitComp)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
	}

	static void AddMenu_TransitToCamera(TSharedRef<FExtender> Extender, UTransitiveCameraActorComponent* Comp)
	{
		Extender->AddMenuExtension(
			"ActorSelectVisibilityLevels",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateStatic(&ContextMenuExtender::AddTransitToCameraMenu, Comp));
		//FMenuExtensionDelegate::CreateRaw(this, &FMatineeToLevelSequenceModule::CreateLevelViewportContextMenuEntries, ActorsToConvert));
	}

	static void GetUniqueStringByUObject(AActor* Actor)
	{
		if (Actor)
		{
			FString str = UGABlueprintFunctionLibrary::GetUniqueStringByUObject(Actor);
			FPlatformApplicationMisc::ClipboardCopy(*str);
		}
	}

	static void AddGetUniqueStringByUObjectMenu(FMenuBuilder& MenuBuilder, AActor* Actor)
	{
		// ~Begin: TransitiveCameraActor
		{
			MenuBuilder.AddMenuSeparator("Editor");
			//MenuBuilder.BeginSection("Editor", FText::FromString("Editor"));
			MenuBuilder.AddMenuEntry(
				FText::FromString("Get Unique String"),
				FText::FromString("To get the unique string of this object"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ContextMenuExtender::GetUniqueStringByUObject, Actor)),
				NAME_None,
				EUserInterfaceActionType::Button
			);
			MenuBuilder.EndSection();
		}
	}

	static void AddMenu_GetUniqueStringByUObject(TSharedRef<FExtender> Extender, AActor* Actor)
	{
		Extender->AddMenuExtension(
			"ActorSelectVisibilityLevels",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateStatic(&ContextMenuExtender::AddGetUniqueStringByUObjectMenu, Actor));
		//FMenuExtensionDelegate::CreateRaw(this, &FMatineeToLevelSequenceModule::CreateLevelViewportContextMenuEntries, ActorsToConvert));
	}

	static TSharedRef<FExtender> ExtendLevelViewportContextMenu(const TSharedRef<FUICommandList> CommandList, const TArray<AActor*> SelectedActors)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		TArray<TWeakObjectPtr<AActor> > ActorsToConvert;
		for (AActor* Actor : SelectedActors)
		{
			AddMenu_GetUniqueStringByUObject(Extender, Actor);
			if (UTransitiveCameraActorComponent* Comp = Actor->FindComponentByClass<UTransitiveCameraActorComponent>())
			{
				AddMenu_TransitToCamera(Extender, Comp);
				break;
			}
		}


		return Extender;
	}
};
