// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CoopGameplayAbility.h"
#include "GA_Gunshot.generated.h"

UCLASS()
class COOPTPS_API UGA_Gunshot : public UCoopGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Gunshot();

	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	UAnimMontage* ShootHipMontage;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	UAnimMontage* ShootAimMontage;

	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;
//Called from the player as ActivateAbilityWithTag()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category = "Game Settings")
	float Range = 1000;
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category = "Game Settings")
	float Damage = 20;

	UFUNCTION()
	void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);
	UFUNCTION()
	void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);
	UFUNCTION()
	void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
};
