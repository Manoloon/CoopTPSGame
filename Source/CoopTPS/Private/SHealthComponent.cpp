// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"



// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	MaxHealth = 100.0f;
	SetIsReplicated (true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only on server --- Como es un componente , no tiene role, sino que su dueño lo tiene.
	if(GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);			
		}
	}	
	Health = MaxHealth;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f) { return; }
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}


/*Este Getter es para poder tomar el dato de la variable HEALTH desde afuera*/
float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::Heal(float HealAmount)
{
	if(HealAmount <=0.0f || Health <=0.0f)
	{
		return;
	}
	Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr,nullptr,nullptr);
	//UE_LOG(LogTemp, Warning, TEXT("Health amount added: %s"), *FString::SanitizeFloat(HealAmount));
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
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health,Damage, nullptr, nullptr, nullptr);
}