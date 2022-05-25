#pragma once
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTT_RunAwayFromTarget.generated.h"

UCLASS()
class COOPTPS_API UBTT_RunAwayFromTargetTask final : public UBTTask_BlueprintBase
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(Category = Blackboard, EditAnywhere)
		FBlackboardKeySelector BBEscapePoint;
	UPROPERTY(Category = Settings, EditAnywhere)
		float Radius =150.0f;
};