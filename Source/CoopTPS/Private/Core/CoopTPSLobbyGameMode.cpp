// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopTPSLobbyGameMode.h"
#include "CoopGameInstance.h"
#include "SCharacter.h"
#include "TPSHud.h"
#include "CoopPlayerController.h"
#include "SPlayerState.h"
#include "TimerManager.h"

ACoopTPSLobbyGameMode::ACoopTPSLobbyGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn>
					BPPlayerCharacterClass(TEXT("/Game/Blueprints/Player_Pawn"));
	if (BPPlayerCharacterClass.Class != nullptr)
	{
		DefaultPawnClass = BPPlayerCharacterClass.Class;
	}
	
	PlayerStateClass = ASPlayerState::StaticClass();
	PlayerControllerClass = ACoopPlayerController::StaticClass();
	HUDClass = ATPSHud::StaticClass();

	PlayerColors.Add(FLinearColor::Blue);
	PlayerColors.Add(FLinearColor::Red);
	PlayerColors.Add(FLinearColor::Yellow);
	PlayerColors.Add(FLinearColor::Green);
	LastPlayerColorIndex = -1;
}

void ACoopTPSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++NumOfPlayers;
	if(NumOfPlayers>=2)
	{
		FTimerHandle StartTravelTH;
		GetWorldTimerManager().SetTimer(StartTravelTH, this,
			&ACoopTPSLobbyGameMode::TravelToMap, 25.0f, false, 1.0f);
	}
}

void ACoopTPSLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumOfPlayers;
}

void ACoopTPSLobbyGameMode::TravelToMap()
{
	if(const auto GameInstance = Cast<UCoopGameInstance>(GetGameInstance()); GameInstance !=nullptr)
	{
		GameInstance->StartSession();
	}
	if(GetWorld()!= nullptr)
	{
		bUseSeamlessTravel = true;
		GetWorld()->ServerTravel("/Game/Map/M_Level?listen");
	}
}

void ACoopTPSLobbyGameMode::SetPlayerDefaults(class APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	if (ASCharacter* CoopPawn = Cast<ASCharacter>(PlayerPawn))
	{
		const int32 PlayerColorIndex = (LastPlayerColorIndex + 1) % PlayerColors.Num();
		if (PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			CoopPawn->AuthSetPlayerColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}
