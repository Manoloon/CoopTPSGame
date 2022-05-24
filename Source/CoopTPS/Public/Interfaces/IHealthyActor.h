#pragma once
#include "CoreMinimal.h"
#include "IHealthyActor.generated.h"

class USHealthComponent;
UINTERFACE(MinimalAPI,meta=(CannotImplementInterfaceInBlueprint))
class UIHealthyActor : public UInterface
{
	GENERATED_BODY()
};

class IIHealthyActor
{
	GENERATED_BODY()
public:
   virtual USHealthComponent* I_GetHealthComp() const=0;
};