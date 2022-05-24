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
	static ConstructorHelpers::FClassFinder<APawn>BPPlayerCharacterClass
										(TEXT("/Game/Blueprints/Player_Pawn"));
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
	ASCharacter* CoopPawn = Cast<ASCharacter>(PlayerPawn);
	if (CoopPawn)
	{
		const int32 PlayerColorIndex = (LastPlayerColorIndex + 1)
												% PlayerColors.Num();
		if (PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			CoopPawn->AuthSetPlayerColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}

void ASGameMode::StartWave()
{
	WaveCount++;
	NumBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this,
		&ASGameMode::SpawnBotTimerElapsed, 1.0f,true,0.0f);
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_SpawnBots);
	SetWaveState(EWaveState::WaveComplete);
}

void ASGameMode::PrepareNextWave()
{
	GetWorldTimerManager().SetTimer(TH_NextWaveStart, this,
			&ASGameMode::StartWave, TimeBetweenWaves, false);
	SetWaveState(EWaveState::WaitingToStart);
	RestoreDeadPlayer();
}

void ASGameMode::CheckWaveState()
{
	const bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TH_NextWaveStart);

	if (NumBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (TActorIterator<APawn>PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		const APawn* TestPawn = *PawnIterator;
		if(TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
		continue;
		}

		const USHealthComponent* HealthComp =Cast<USHealthComponent>
			(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() >0.0f)
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
		const	APlayerController* PC = It->Get();
		if(PC && PC->GetPawn())
		{
			const APawn* PlayerPawn = PC->GetPawn();
			const USHealthComponent* HealthComp = Cast<USHealthComponent>
			(PlayerPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			//TODO hacer un null system asi no se corta el juego.
			if(HealthComp && HealthComp->Health > 0.0f )
			{
				return;
			}
		}
	}
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

void ASGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// TODO : send a notification for this , not using tick-
	CheckWaveState();
	CheckAnyPlayerAlive();
}

