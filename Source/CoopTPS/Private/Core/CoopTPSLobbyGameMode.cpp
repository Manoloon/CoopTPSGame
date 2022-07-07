// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CoopTPSLobbyGameMode.h"
#include "Core/CoopGameInstance.h"
#include "Entities/SCharacter.h"
#include "Core/TPSHud.h"
#include "Core/CoopPlayerController.h"
#include "Core/SPlayerState.h"
#include "TimerManager.h"

ACoopTPSLobbyGameMode::ACoopTPSLobbyGameMode()
{
	if (static ConstructorHelpers::FClassFinder<APawn> BPPlayerCharacterClass(TEXT("/Game/Blueprints/Player_Pawn"));
		BPPlayerCharacterClass.Class != nullptr)
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
	if(NumOfPlayers>=MaxNumPlayers)
	{
		if(FTimerHandle Th_StartTravel;
			!GetWorldTimerManager().IsTimerActive(Th_StartTravel))
		{
			GetWorldTimerManager().SetTimer(Th_StartTravel, this,
				&ACoopTPSLobbyGameMode::TravelToMap, 25.0f, false, 1.0f);
		}
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
	if(GetWorld())
	{
		bUseSeamlessTravel = true;
		UE_LOG(LogTemp, Warning, TEXT("OnTravelToMap"));
		GetWorld()->ServerTravel(FString("/Game/Map/M_Level?listen"));
	}
}

void ACoopTPSLobbyGameMode::SetPlayerDefaults(class APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	if (ASCharacter* CoopPawn = Cast<ASCharacter>(PlayerPawn))
	{
		if (const int32 PlayerColorIndex = (LastPlayerColorIndex + 1) % PlayerColors.Num();
			PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			CoopPawn->AuthSetPlayerColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}
