// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CoopAttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

UCLASS()
class COOPTPS_API ASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	UPROPERTY()
	class ASCharacter* PlayerCharacter;
	UPROPERTY()
	class ACoopPlayerController* PlayerController;
	
public:
	ASPlayerState();
	virtual void OnRep_Score() override;
	UFUNCTION(BlueprintCallable)
		void AddToScore(const float ScoreDelta);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UCoopAttributeSet* GetAttributeSet() const;
protected:
	FGameplayTag DeadTag;
	FGameplayTag KnockedDownTag;

	UPROPERTY()
		UAbilitySystemComponent* AbilitySystemComponent;
	UPROPERTY()
		class UCoopAttributeSet* AttributeSetBase;
};
