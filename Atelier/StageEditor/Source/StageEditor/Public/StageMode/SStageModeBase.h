#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DeclarativeSyntaxSupport.h"
#include "Framework/Commands/Commands.h"

class SStageModeBase : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStageModeBase) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);

	virtual FString GetName() = 0;
	virtual FString GetDescription() = 0;
protected:
};