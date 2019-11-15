#include "ActionEventGraphSchema.h"

#define LOCTEXT_NAMESPACE "ActionEvent"

FEventGraphSchemaAction::FEventGraphSchemaAction(TSharedPtr<struct FActionEvent> InActionEvent)
{
	check(InActionEvent.IsValid());

	ActionEventPtr = InActionEvent;
	//Grouping = 0;
	//SectionID = 0;

	UpdateSearchData(GetEventName(), GetEventName(), GetEventCategory(), FText());
	ActionEventPtr->OnEventDataDirty.BindRaw(this, &FEventGraphSchemaAction::OnActionEventBeDirty);
}

FText FEventGraphSchemaAction::GetEventCategory()
{
	return FText::FromName("All");
}

FText FEventGraphSchemaAction::GetEventName()
{
	return FText::FromString(FName::NameToDisplayString(ActionEventPtr->Name.ToString() + "(" + FString::FromInt(ActionEventPtr->Id) + (ActionEventPtr->IsDirty() ? ") *" : ")"), false));
}

void FEventGraphSchemaAction::OnActionEventBeDirty(bool bDirty)
{
	UpdateSearchData(GetEventName(), GetEventName(), GetEventCategory(), FText());
}

#undef LOCTEXT_NAMESPACE
