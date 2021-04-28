// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopTPSLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API ACoopTPSLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;

private:
	uint32 NumOfPlayers = 0;
	void TravelToMap();	
};
