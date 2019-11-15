// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-06-05
#include "Sections/MovieSceneStateSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "SequencerObjectVersion.h"
#include "Internationalization.h"
#include "ModuleManager.h"
#include "SlateApplication.h"
#include "DeclarativeSyntaxSupport.h"
#include "MovieSceneStateNotify.h"
#include "SharedPointer.h"
#include "UIAction.h"
#include "STextEntryPopup.h"
#include "SlateApplication.h"
#include "SEditableText.h"
#include "Text.h"
#include "MultiBoxBuilder.h"

#if WITH_EDITORONLY_DATA
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#endif

bool FMovieSceneStateSectionData::Evaluate(FFrameTime InTime, FStatePayload& OutValue) const
{
	return true;
}
void FMovieSceneStateSectionData::GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FMovieSceneStateSectionData::GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
}

void FMovieSceneStateSectionData::SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FMovieSceneStateSectionData::DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FMovieSceneStateSectionData::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	GetData().DeleteKeys(InHandles);
}

void FMovieSceneStateSectionData::ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FMovieSceneStateSectionData::ComputeEffectiveRange() const
{
	return GetData().GetTotalRange();
}

int32 FMovieSceneStateSectionData::GetNumKeys() const
{
	return Times.Num();
}

void FMovieSceneStateSectionData::Reset()
{
	Times.Reset();
	Values.Reset();
	KeyHandles.Reset();
}

void FMovieSceneStateSectionData::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

void FMovieSceneStateSectionData::Optimize(const FKeyDataOptimizationParams& InParameters)
{
}

void FMovieSceneStateSectionData::ClearDefault()
{
}

void FMovieSceneStateSectionData :: NotifyAll(AActor* owner, int32 index) const
{
	for (int i = 0; i <  Values[index].NotifyList.Num(); i++)
	{
		Values[index].NotifyList[i]->Notify(owner);
	}
}

TArray<UMovieSceneStateNotify*> FMovieSceneStateSectionData::GetNotifyList(int32 index)
{
	return Values[index].NotifyList;
}

void FMovieSceneStateSectionData::AddNotifyToData(int32 index, UClass* inclass)
{
	class UObject* NotifyClass = NewObject<UObject>(CurrentMovieScene, inclass, NAME_None, RF_Transactional);
	Values[index].NotifyList.Add(Cast<UMovieSceneStateNotify>(NotifyClass));
}

void FMovieSceneStateSectionData::RemoveNotifyToData(int32 index, int32 offset)
{
	if (offset < Values[index].NotifyList.Num()) 
	{
		Values[index].NotifyList.RemoveAt(offset);
	}
}

void FMovieSceneStateSectionData::ReplaceNotifyToData(int32 index, int32 offset, UClass* inclass)
{
	if (offset < Values[index].NotifyList.Num())
	{
		class UObject* NotifyClass = NewObject<UObject>(CurrentMovieScene, inclass, NAME_None, RF_Transactional);
		Values[index].NotifyList[offset] = Cast<UMovieSceneStateNotify>(NotifyClass);
	}
}

void FMovieSceneStateSectionData::DrawNotifyInfo(FMenuBuilder& MenuBuilder, FKeyHandle KeyHandle, UMovieScene* moviescene)
{
	CurrentMovieScene = moviescene;
	int32 index = GetData().GetIndex(KeyHandle);
	TArray<UMovieSceneStateNotify*> notifylist = GetNotifyList(index);
	MenuBuilder.BeginSection("AnimNotify", FText::FromString("Notify"));
	{
		for (int i = 0; i < notifylist.Num(); i++)
		{
			UMovieSceneStateNotify* notify = notifylist[i];
			FString name = notify->GetNotifyName();
			MenuBuilder.AddSubMenu(
				FText::FromString(name),
				FText::FromString("Modify this notify"),
				FNewMenuDelegate::CreateRaw(this, &FMovieSceneStateSectionData::ModifyNotify, index, i)
			);
		}
		MenuBuilder.AddSubMenu(
			FText::FromString("Add Notify"),
			FText::FromString("Add notify to this state"),
			FNewMenuDelegate::CreateRaw(this, &FMovieSceneStateSectionData::AddNotify, index)
		);
	}
	MenuBuilder.EndSection();
}

void FMovieSceneStateSectionData::AddNotify(FMenuBuilder& MenuBuilder, int32 index)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("New Notify..."), 
		FText::FromString("Create a new animation notify on the skeleton"), 
		FSlateIcon(), 
		FUIAction(FExecuteAction::CreateRaw(this, &FMovieSceneStateSectionData::OnNewNotifyClicked))
	);
	MakeNewNotifyPicker(MenuBuilder, false, index, 0);
}

void FMovieSceneStateSectionData::ModifyNotify(FMenuBuilder& MenuBuilder, int32 index, int32 offset)
{
	MenuBuilder.BeginSection("AnimNotifySkeletonSubMenu", FText::FromString(""));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Delete"),
			FText::FromString("Delete the notify from this state"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FMovieSceneStateSectionData::RemoveNotifyToData, index, offset))
		);
		MakeNewNotifyPicker(MenuBuilder, true, index, offset);
	}
	MenuBuilder.EndSection();
}

void FMovieSceneStateSectionData::OnNewNotifyClicked()
{
	//Show dialog to enter new track name
	TSharedRef<STextEntryPopup> TextEntry = 
		SNew(STextEntryPopup)
		.Label(FText::FromString("Notify Name"))
		.OnTextCommitted_Raw(this, &FMovieSceneStateSectionData::CreateNewNotify);

	TSharedPtr<SWindow> Parent = FSlateApplication::Get().GetActiveTopLevelWindow();
	FSlateApplication::Get().PushMenu(
		Parent.ToSharedRef(),
		FWidgetPath(),
		TextEntry,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
	);
}

void FMovieSceneStateSectionData::MakeNewNotifyPicker(FMenuBuilder& MenuBuilder, bool bIsReplaceWithMenu, int32 index, int32 offset)
{
#if WITH_EDITORONLY_DATA
	class FNotifyStateClassFilter : public IClassViewerFilter
	{
	public:
		FNotifyStateClassFilter() {}

		bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			const bool bChildOfObjectClass = InClass->IsChildOf(UMovieSceneStateNotify::StaticClass());
			const bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
			bool HasSame = false;
			return bChildOfObjectClass && bMatchesFlags && CastChecked<UMovieSceneStateNotify>(InClass->ClassDefaultObject);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			const bool bChildOfObjectClass = InUnloadedClassData->IsChildOf(UMovieSceneStateNotify::StaticClass());
			const bool bMatchesFlags = !InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
			bool bValidToPlace = false;
			if (bChildOfObjectClass)
			{
				if (const UClass* NativeBaseClass = InUnloadedClassData->GetNativeParent())
				{
					bValidToPlace = CastChecked<UMovieSceneStateNotify>(NativeBaseClass->ClassDefaultObject);
				}
			}
			return bChildOfObjectClass && bMatchesFlags && bValidToPlace;
		}
	};

	if (MenuBuilder.GetMultiBox()->GetBlocks().Num() > 1)
	{
		MenuBuilder.AddMenuSeparator();
	}

	FClassViewerInitializationOptions InitOptions;
	InitOptions.Mode = EClassViewerMode::ClassPicker;
	InitOptions.bShowObjectRootClass = false;
	InitOptions.bShowUnloadedBlueprints = true;
	InitOptions.bShowNoneOption = false;
	InitOptions.bEnableClassDynamicLoading = true;
	InitOptions.bExpandRootNodes = true;
	InitOptions.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	InitOptions.ClassFilter = MakeShared<FNotifyStateClassFilter>();
	InitOptions.bShowBackgroundBorder = false;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	MenuBuilder.AddWidget(
		SNew(SBox)
		.MinDesiredWidth(300.0f)
		.MaxDesiredHeight(400.0f)
		[
			ClassViewerModule.CreateClassViewer(
				InitOptions,
				FOnClassPicked::CreateLambda([this, bIsReplaceWithMenu, index, offset](UClass* InClass)
				{
					FSlateApplication::Get().DismissAllMenus();
					if (bIsReplaceWithMenu)
					{
						ReplaceNotifyToData(index, offset, InClass);
					}
					else
					{
						AddNotifyToData(index, InClass);
					}
				})
			)
		],
		FText(), true, false
	);
#endif
}

void FMovieSceneStateSectionData::CreateNewNotify(const FText& NewNotifyName, ETextCommit::Type CommitInfo)
{
	FSlateApplication::Get().DismissAllMenus();
	UE_LOG(LogTemp, Error, TEXT("Todo: CreateNewNotify blueprint by c++"));
}

UMovieSceneStateSection::UMovieSceneStateSection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bSupportsInfiniteRange = true;
	// No Need
	EvalOptions.EnableAndSetCompletionMode
	(GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToRestoreState ?
		EMovieSceneCompletionMode::KeepState :
		GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToProjectDefault ?
		EMovieSceneCompletionMode::RestoreState :
		EMovieSceneCompletionMode::ProjectDefault);

#if WITH_EDITOR

	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(StateData, FMovieSceneChannelMetaData());

#else

	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(StateData);

#endif
}
