#include "ActionSkillTypes.h"
#include "ActionSkill.h"
#include "MessageDialog.h"
#include "ActionEvent.h"

#if WITH_EDITOR
int32 NextValidEventId = 0;
#endif

const FName EVENT_PROP_ID = "Id";

/*********************************************************************/
// FActionKeyFrame
/*********************************************************************/
FActionKeyFrame::FActionKeyFrame() 
    : Time(0.f), EventId(INDEX_NONE)
{
#if WITH_EDITOR
    EventArgs.SetUserStructBaseType(UActionEventArgsStruct::StaticClass());
#endif
}

FActionEventPtr FActionKeyFrame::GetEvent() const
{
    return FActionSkillModule::Get().GetActionEventById(EventId);
}

/*********************************************************************/
// FActionEvent
/*********************************************************************/
#if WITH_EDITOR
void FActionEvent::SetId(int32 InId)
{
	Id = InId;
}

void FActionEvent::MarkDirty(bool bIsDirty, UProperty* DirtyProperty)
{
	bDirty = bIsDirty;
	OnEventDataDirty.ExecuteIfBound(bDirty);

	if (bIsDirty)
	{
		if (DirtyProperty)
		{
			UStruct* owner = DirtyProperty->GetOwnerStruct();
			if (owner->GetFName() == "ActionEvent" && DirtyProperty->GetName() == "Id")
			{
				OnEventIdChanged.ExecuteIfBound(*this);
			}
		}
	}
}
#endif
/*********************************************************************/
// UActionEventTable
/*********************************************************************/
UActionEventTable::UActionEventTable(const FObjectInitializer& ObjectInitializer)
{
    ActionEvents.Empty();
}

void UActionEventTable::EmptyTable()
{
    ActionEvents.Empty();
}

#if WITH_EDITOR
FActionEventPtr UActionEventTable::NewEvent()
{
    FActionEventPtr NewEvent = MakeShareable(new FActionEvent());
	static UScriptStruct* EventStruct = FActionEvent::StaticStruct();
	//NewEvent->Initialize(*EventStruct);
	NewEvent->OnEventIdChanged.BindUObject(this, &UActionEventTable::OnEventIdChanged);

    NewEvent->SetId(NextValidEventId);
    UpdateNextValideId(NewEvent->Id);
	NewEvent->MarkDirty(true);
    ActionEvents.Add(NewEvent->Id, NewEvent);

    return NewEvent;
}

void UActionEventTable::DeleteEvent(int32 Id)
{
    ActionEvents.Remove(Id);
}

void UActionEventTable::UpdateNextValideId(int32 NewId)
{
	if (NewId >= NextValidEventId)
	{
		NextValidEventId = NewId + 1;
	}
}

void UActionEventTable::OnEventIdChanged(FActionEvent & InEvent)
{
	int32 OldId = INDEX_NONE;
	int32 NewId = InEvent.Id;

	for (EventIterator It = CreateIterator(); It; ++It)
	{
		if (It->Value.Get() == &InEvent && It->Key != NewId)
		{
			OldId = It->Key;
			break;
		}
	}

	if (OldId != INDEX_NONE)
	{
		if (ActionEvents.Contains(NewId))
		{
			EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
				FString::Printf(TEXT("Duplicated Id (%d) on events {%s, %s}"),
					NewId,
					*ActionEvents.Find(NewId)->Get()->Name.ToString(),
					*InEvent.Name.ToString()
				))
			);
			InEvent.SetId(OldId);
			InEvent.OnEventDataDirty.ExecuteIfBound(true);
			return;
		}

		FActionEventPtr EventToMove;
		if (ActionEvents.RemoveAndCopyValue(OldId, EventToMove))
		{
			ActionEvents.Add(NewId, EventToMove);
		}
	}

	UpdateNextValideId(NewId);
	return;
}

void UActionEventTable::OnSaveData()
{
    for (auto Pair : ActionEvents)
    {
        Pair.Value->MarkDirty(false);
    }
}
#endif // WITH_EDITOR