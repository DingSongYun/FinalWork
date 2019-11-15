// Copyright 2019 X.D., Inc. All Rights Reserved.
// Author: SongYun.Ding
// Date: 2019-05-31

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "StateSequenceFactory.generated.h"

/**
 * Implements a factory for ULevelSequence objects.
 */
UCLASS(hidecategories=Object)
class STATESEQUENCEEDITOR_API UStateSequenceFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	// UFactory Interface

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};

