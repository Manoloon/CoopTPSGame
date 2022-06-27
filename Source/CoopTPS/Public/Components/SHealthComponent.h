// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, class USHealthComponent*, 
HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType,
										class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(CoopTPS), meta=(BlueprintSpawnableComponent) )
class COOPTPS_API USHealthComponent final : public UActorComponent
{
	GENERATED_BODY()

public:	
	USHealthComponent()=default;
	
	UPROPERTY(ReplicatedUsing=ONREP_Health, BlueprintReadOnly, Category = "Settings")
		float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
		bool bCanAutoHeal=false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
		float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
		uint8 TeamNum = 255;

	bool bOwnerIsDead=false;
	float GetHealth() const;
	UPROPERTY(BlueprintAssignable, Category = "events")
    	FOnHealthChangedSignature OnHealthChanged;
    UFUNCTION(BlueprintCallable, Category = "Health")
    	void Heal(const float HealAmount);
	
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Team")
    	static bool IsFriendly(AActor* ActorA , AActor* ActorB);
	
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	                                           class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
		void ONREP_Health(float OldHealth);	
};


