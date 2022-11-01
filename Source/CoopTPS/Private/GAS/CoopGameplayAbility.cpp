// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CoopGameplayAbility.h"

#include "AbilitySystemComponent.h"

UCoopGameplayAbility::UCoopGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun")));
}

void UCoopGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	if(ActivateAbilityOnGranted)
	{
		[[maybe_unused]] bool ActivatedAbility = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle,false);
	}
}

