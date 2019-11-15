#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "ActionEvent.h"
#include "ActionEventGraphSchema.generated.h"

USTRUCT()
struct ACTIONSKILLEDITOR_API FEventGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	// Simple type info
	static FName StaticGetTypeId() { static FName Type("FEventGraphSchemaAction"); return Type; }
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	FEventGraphSchemaAction()
		: FEdGraphSchemaAction()
	{}

	FEventGraphSchemaAction(TSharedPtr<struct FActionEvent> InActionEvent);

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override
	{
		return nullptr;
	}
	//~ End FEdGraphSchemaAction Interface

	FText GetEventCategory();
	FText GetEventName();

private:
	void OnActionEventBeDirty(bool bDirty);

public:
	/** Pointer to inner ActionSkill */
	TSharedPtr<struct FActionEvent> ActionEventPtr;
};