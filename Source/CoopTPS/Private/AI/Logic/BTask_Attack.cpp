// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Logic/BTask_Attack.h"
#include "CoopAIController.h"
#include "IInputComm.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AController* MyController = Cast<AController>(OwnerComp.GetOwner());
	//APawn* MyPawn = MyController ? MyController->GetPawn() : NULL;
	if(MyController->GetPawn() && MyController->GetPawn()->Implements<UIInputComm>())
	{
		const auto I = Cast<IIInputComm>(MyController->GetPawn());
		I->I_StartFire();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
