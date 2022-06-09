// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SHealthComponent.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Core/SGameMode.h"

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);
	// Only on server --- Como es un componente , no tiene role, sino que su dueño lo tiene.
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
	// if friendly , no damage, But can do SelfDamage (trackball self destruct)
	if(DamagedActor != DamageCauser && IsFriendly(DamagedActor,DamageCauser))
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	bOwnerIsDead = Health <= 0.0f;

	if (bOwnerIsDead)
	{
		if (const ASGameMode* GamMode =Cast<ASGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GamMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser,InstigatedBy);
		}
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
		return true;
	}
	const USHealthComponent* HealthCompA = Cast<IIHealthyActor>(ActorA)->I_GetHealthComp();
	const USHealthComponent* HealthCompB = Cast<IIHealthyActor>(ActorB)->I_GetHealthComp();
	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

/*NETWORKING*/

// networking
void USHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME(USHealthComponent, Health);
}

void USHealthComponent::ONREP_Health(float OldHealth)
{
	const float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health,Damage, nullptr, nullptr, nullptr);
}