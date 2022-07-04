// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/SPlayerState.h"
#include "Core/CoopPlayerController.h"
#include "Entities/SCharacter.h"

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
			PlayerController->SetHUDScore(GetScore());
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
			PlayerController->SetHUDScore(GetScore());
		}
	}
}
