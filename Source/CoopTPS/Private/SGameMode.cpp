// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "SCharacter.h"
#include "CoopPlayerController.h"
#include "TPSHud.h"
#include "SHealthComponent.h"
#include "Engine/World.h" // FConstIterators
#include "SGameState.h"
#include "EngineUtils.h"
#include "SPlayerState.h"
#include "TimerManager.h"


ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;

	// declaramos al SGAMESTATE como el default.
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	PlayerControllerClass = ACoopPlayerController::StaticClass();
	DefaultPawnClass = ASCharacter::StaticClass();
	HUDClass = ATPSHud::StaticClass();	

	// colores
	PlayerColors.Add(FLinearColor::Blue);
	PlayerColors.Add(FLinearColor::Red);
	PlayerColors.Add(FLinearColor::Yellow);
	PlayerColors.Add(FLinearColor::Green);
	LastPlayerColorIndex = -1;

	// prepara el tick para correr cada segundo y no cada frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}



void ASGameMode::StartWave()
{
	WaveCount++;
	NumBotsToSpawn = 2 * WaveCount;
	// start timer
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f,true,0.0f);
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_SpawnBots);
	SetWaveState(EWaveState::WaveComplete);
}

void ASGameMode::PrepareNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
	SetWaveState(EWaveState::WaitingToStart);
	RestoreDeadPlayer();
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	/*SI todavia quedan bots en pantalla O se esta preparando para la proxima oleada , no correr nada de abajo*/
	if (NumBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	// con esta sintaxis chequea todos los pawns que hay en escena
	for (TActorIterator<APawn>PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		APawn* TestPawn = *PawnIterator;
		if(TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
		continue;
		}

		USHealthComponent* HealthComp =Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() >0.0f) // if still alive
		{
			bIsAnyBotAlive = true;
			break;
		}
	}
	if (!bIsAnyBotAlive)
	{
		PrepareNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		APlayerController* PC = It->Get();
		if(PC && PC->GetPawn())
		{
			APawn* PlayerPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(PlayerPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			// nos aseguramos que esto funcione ,sino ENSURE hace un break para avisar que algo esta muy mal.
			if(ensure(HealthComp && HealthComp->Health > 0.0f ))
			{
				return;
			}
		}
	}
	// no player alive!	
	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
}

void ASGameMode::SetWaveState(EWaveState NewWaveState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewWaveState);
	}
}

void ASGameMode::RestoreDeadPlayer()
{
	for (FConstPlayerControllerIterator It= GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		APlayerController* PC = It->Get();
		if(PC && PC->GetPawn() == nullptr)
		{
			// this function is god! jaja
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareNextWave();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumBotsToSpawn--;
	if(NumBotsToSpawn <=0)
	{
		EndWave();
	}
}

/*TICKING*/
void ASGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASGameMode::SetPlayerDefaults(class APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	ASCharacter* CoopPawn = Cast<ASCharacter>(PlayerPawn);
	if (CoopPawn)
	{
		const int32 PlayerColorIndex = (LastPlayerColorIndex + 1) % PlayerColors.Num();
		if(PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			CoopPawn->AuthSetPlayerColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}

