#pragma once
#include "CoreMinimal.h"
#include "IInteractiveActor.generated.h"

UINTERFACE(MinimalAPI,meta=(CannotImplementInterfaceInBlueprint))
class UIInteractiveActor : public UInterface
{
	GENERATED_BODY()
};

class IIInteractiveActor
{
	GENERATED_BODY()
public:
   virtual FLinearColor GetActorTypeColor()=0;
};