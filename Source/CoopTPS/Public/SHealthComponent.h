// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(CoopTPS), meta=(BlueprintSpawnableComponent) )
class COOPTPS_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();
	UPROPERTY(ReplicatedUsing=ONREP_Health, BlueprintReadOnly, Category = "Health")
		float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
		float MaxHealth;

	bool bOwnerIsDead;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UFUNCTION() // muy importante poner esto si la funcion se usara de delegate. Sino tirara error
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
		void ONREP_Health(float OldHealth);

public:
	/*se usa para retornar el valor float de la variable health desde afuera*/
	float GetHealth() const;

	UPROPERTY(BlueprintAssignable, Category = "events")
		FOnHealthChangedSignature OnHealthChanged;
	UFUNCTION(BlueprintCallable, Category = "Health")
		void Heal(float HealAmount);
};
