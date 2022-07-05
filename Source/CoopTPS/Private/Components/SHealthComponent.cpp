// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SHealthComponent.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Core/SGameMode.h"
#include "Kismet/GameplayStatics.h"

static bool DebugHealth = false;
FAutoConsoleVariableRef CVarDebugHealthComponent(TEXT("COOP.Health"),
DebugHealth,TEXT("Health component debug"),ECVF_Cheat);

USHealthComponent::USHealthComponent()
{
	SetIsReplicatedByDefault(true);
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwnerRole() == ROLE_Authority)
	{
		if (AActor* MyOwner = GetOwner())
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);			
		}
	}	
	Health = MaxHealth;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bOwnerIsDead) {return; }
	if(DamagedActor != DamageCauser && IsFriendly(DamagedActor,DamageCauser))
	{
		UE_LOG(LogTemp,Display,TEXT("%s : IS FRIENDLY"),*this->GetName());
		//TODO : Hint about friendly if the damageCauser is pawn controlled player.
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	bOwnerIsDead = Health <= 0.0f;
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy,
																				DamageCauser);
	if (bOwnerIsDead)
	{
		if (const ASGameMode* GamMode =Cast<ASGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GamMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser,
						GetOwner()->GetInstigatorController(),InstigatedBy);
		}
	}
	else if(bCanAutoHeal)
	{
		// refill the health by seconds.
	}
}

float USHealthComponent::GetHealth() const{	return Health; }

void USHealthComponent::Heal(const float HealAmount)
{
	if(HealAmount <=0.0f || bOwnerIsDead){return;}
	Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, -HealAmount,
									nullptr,nullptr,nullptr);
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if(ActorA == nullptr || ActorB == nullptr)
	{
		return true;
	}
	if(!ActorA->Implements<UIHealthyActor>() || !ActorB->Implements<UIHealthyActor>())
	{
		return false;
	}
	const USHealthComponent* HealthCompA = Cast<IIHealthyActor>(ActorA)->I_GetHealthComp();
	const USHealthComponent* HealthCompB = Cast<IIHealthyActor>(ActorB)->I_GetHealthComp();
	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

/*NETWORKING*/
void USHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent, Health);
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	const float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health,Damage, nullptr,nullptr, nullptr);
}

