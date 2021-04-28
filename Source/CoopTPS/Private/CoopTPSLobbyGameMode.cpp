// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopTPSLobbyGameMode.h"
#include "CoopGameInstance.h"
#include "TimerManager.h"

void ACoopTPSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++NumOfPlayers;
	if(NumOfPlayers>=2)
	{
		FTimerHandle StartTravelTH;
		GetWorldTimerManager().SetTimer(StartTravelTH, this, &ACoopTPSLobbyGameMode::TravelToMap, 10.0f, false, 1.0f);
	}
}

void ACoopTPSLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumOfPlayers;
}

void ACoopTPSLobbyGameMode::TravelToMap()
{
	auto GameInstance = Cast<UCoopGameInstance>(GetGameInstance());
	if(GameInstance !=nullptr)
	{
		GameInstance->StartSession();
	}
	UWorld* World = GetWorld();
	if(World != nullptr)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel("/Game/Map/M_Level?listen");
	}
}
