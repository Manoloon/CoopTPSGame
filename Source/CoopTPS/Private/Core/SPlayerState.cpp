// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/SPlayerState.h"

#include "AbilitySystemComponent.h"
#include "CoopAttributeSet.h"
#include "Core/CoopPlayerController.h"
#include "Entities/SCharacter.h"

ASPlayerState::ASPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSetBase = CreateDefaultSubobject<UCoopAttributeSet>(TEXT("AttributeSetBase"));

	NetUpdateFrequency = 100.0f;

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
}

void ASPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	PlayerCharacter = (PlayerCharacter == nullptr)? Cast<ASCharacter>(GetPawn()) : PlayerCharacter;
	if(PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr)? Cast<ACoopPlayerController>(PlayerCharacter->GetController())
														: PlayerController;
		if(PlayerController)
		{
			PlayerController->SetHudScore(GetScore());
		}
	}
}

void ASPlayerState::AddToScore(const float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);
	PlayerCharacter = (PlayerCharacter == nullptr)? Cast<ASCharacter>(GetPawn()) : PlayerCharacter;
	if(PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr)? Cast<ACoopPlayerController>(PlayerCharacter->GetController())
														: PlayerController;
		if(PlayerController)
		{
			PlayerController->SetHudScore(GetScore());
		}
	}
}

UAbilitySystemComponent* ASPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UCoopAttributeSet* ASPlayerState::GetAttributeSet() const
{
	return AttributeSetBase;
}
