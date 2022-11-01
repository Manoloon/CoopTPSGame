// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Gunshot.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

UGA_Gunshot::UGA_Gunshot()
{
	AbilityInputID = EGSAbilityInputID::PrimaryFire;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.PrimaryShot")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.PrimaryShot")));
	
	//ActivationBlockedTags.AddTag(GunshotTag);
}

void UGA_Gunshot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if(!CommitAbility(Handle,ActorInfo,ActivationInfo))
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
	}
	//const UAnimMontage* MontageToPlay = ShootHipMontage;
	if(GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights")))
		&& !GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights.Removal"))))
	{
		//MontageToPlay = ShootAimMontage;
	}
	UE_LOG(LogTemp,Warning,TEXT("PEW PEW PEW"));
}

void UGA_Gunshot::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Gunshot::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Gunshot::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// we rely on the montage to shoot the event 
	if(GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority &&
		EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.SpawnProjectile")))
	{
		const ACharacter* Player = Cast<ACharacter>(GetOwningActorFromActorInfo());
		if(!Player)
		{
			EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
		}
		// Linetrace calculation (use range as parameter)
		UE_LOG(LogTemp,Warning,TEXT("PEW PEW "));
		const FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect,GetAbilityLevel());
		// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
	}
}
