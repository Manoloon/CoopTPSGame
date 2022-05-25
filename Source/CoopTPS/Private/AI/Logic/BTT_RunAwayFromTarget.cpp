#include "AI/Logic/BTT_RunAwayFromTarget.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

EBTNodeResult::Type UBTT_RunAwayFromTargetTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                            uint8* NodeMemory)
{
	const AController* MyController = Cast<AController>(OwnerComp.GetOwner());
	UBlackboardComponent* OwnedBlackBoard = OwnerComp.GetBlackboardComponent();
	if(MyController->GetPawn())
	{
		if (const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>
																					(GetWorld()))
		{
			const FVector PawnLoc = MyController->GetPawn()->GetActorLocation();
			if(FNavLocation NavLocation; NavSys->GetRandomReachablePointInRadius(PawnLoc,Radius,NavLocation))
			{
				const FVector RandomLocation = NavLocation.Location;
				if(IsValid(UGameplayStatics::GetPlayerCharacter(GetWorld(),
													0)))
				{
					const FVector PlayerLoc = UGameplayStatics::GetPlayerCharacter(GetWorld(),
									0)->GetActorLocation();
					const FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(PawnLoc,
					PlayerLoc);
					const FVector RandomExit = RandomLocation *
						UKismetMathLibrary::GetForwardVector(LookRot)  + RandomLocation;
					OwnedBlackBoard->SetValueAsVector(BBEscapePoint.SelectedKeyName, RandomExit);
					return EBTNodeResult::Succeeded;
				}
				OwnedBlackBoard->SetValueAsVector(BBEscapePoint.SelectedKeyName, 
											MyController->GetPawn()->GetActorLocation());
				return EBTNodeResult::Failed;
			}
			OwnedBlackBoard->SetValueAsVector(BBEscapePoint.SelectedKeyName, 
										MyController->GetPawn()->GetActorLocation());
			return EBTNodeResult::Failed;
		}
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;
}
