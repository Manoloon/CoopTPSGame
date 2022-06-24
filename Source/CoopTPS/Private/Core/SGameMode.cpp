// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/SGameMode.h"
#include "Entities/SCharacter.h"
#include "Core/CoopPlayerController.h"
#include "Core/TPSHud.h"
#include "Components/SHealthComponent.h"
#include "Engine/World.h" // FConstIterators
#include "Core/SGameState.h"
#include "EngineUtils.h"
#include "Core/SPlayerState.h"
#include "TimerManager.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 5.0f;
	static ConstructorHelpers::FClassFinder<APawn>BPPlayerCharacterClass(TEXT("/Game/Blueprints/Player_Pawn"));
	if(BPPlayerCharacterClass.Class != nullptr)
	{
		DefaultPawnClass = BPPlayerCharacterClass.Class;
	}
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	PlayerControllerClass = ACoopPlayerController::StaticClass();
	HUDClass = ATPSHud::StaticClass();	
	
	PlayerColors.Add(FLinearColor::Blue);
	PlayerColors.Add(FLinearColor::Red);
	PlayerColors.Add(FLinearColor::Yellow);
	PlayerColors.Add(FLinearColor::Green);
	LastPlayerColorIndex = -1;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASGameMode::SetPlayerDefaults(class APawn* PlayerPawn)
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

void ASGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// TODO : send a notification for this , not using tick- maybe dispatcher from entities.
	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
	//TODO : show sign to all players about the count
	WaveCount++;
	NumBotsToSpawn = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(TH_SpawnBots, this,&ASGameMode::SpawnBotTimerElapsed, 1.0f,
																						true,0.0f);
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	//TODO : show sign to all players about stats
	GetWorldTimerManager().ClearTimer(TH_SpawnBots);
	SetWaveState(EWaveState::WaveComplete);
}

void ASGameMode::RestartWave()
{
	GetWorldTimerManager().ClearTimer(TH_SpawnBots);	
}

void ASGameMode::PrepareNextWave()
{
	UE_LOG(LogTemp,Warning,TEXT("Preparing next wave"));
	GetWorldTimerManager().SetTimer(TH_NextWaveStart, this,
			&ASGameMode::StartWave, TimeBetweenWaves, false);
	SetWaveState(EWaveState::WaitingToStart);
	RestoreDeadPlayer();
}

void ASGameMode::CheckWaveState()
{
	if (const bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TH_NextWaveStart);
		NumBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}
	// TODO : better solution to this iterations?
	if (!IsAnyEnemyAlive() && IsAnyPlayerAlive())
	{
		PrepareNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	if(!IsAnyPlayerAlive())
	{
		GameOver();
	}	
}

void ASGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
	UE_LOG(LogTemp,Warning,TEXT("GAME OVER"));
}

void ASGameMode::SetWaveState(EWaveState NewWaveState)
{
	if (ASGameState* GS = GetGameState<ASGameState>();
		ensureAlways(GS))
	{
		GS->SetWaveState(NewWaveState);
	}
}

void ASGameMode::RestoreDeadPlayer()
{
	for (FConstPlayerControllerIterator It= GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		if(APlayerController* PC = It->Get(); PC && PC->GetPawn() == nullptr)
		{
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

bool ASGameMode::IsAnyPlayerAlive() const
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		if(const APlayerController* PC = It->Get(); PC && PC->GetPawn())
		{
			const APawn* PlayerPawn = PC->GetPawn();
			//TODO hacer un null system asi no se corta el juego.
			if(const IIHealthyActor* I = Cast<IIHealthyActor>(PlayerPawn);
				I->I_GetHealthComp()->GetHealth() > 0.0f )
			{
				return true;
			}
		}
	}
	return false;
}

bool ASGameMode::IsAnyEnemyAlive() const
{
	for (TActorIterator<APawn>PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		const APawn* TestPawn = *PawnIterator;
		if(TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}
		if(const IIHealthyActor* I = Cast<IIHealthyActor>(TestPawn);I->I_GetHealthComp()->GetHealth() > 0.0f)
		{
			return true;
		}
	}
	return false;
}

void ASGameMode::DestroyAllEnemies()
{
}
