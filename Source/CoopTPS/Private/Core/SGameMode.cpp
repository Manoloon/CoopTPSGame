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
	static ConstructorHelpers::FClassFinder<ATPSHud>BPTpsHUDClass(TEXT("/Game/Blueprints/BP_TPSHUD"));
	if(BPPlayerCharacterClass.Class != nullptr)
	{
		DefaultPawnClass = BPPlayerCharacterClass.Class;
	}
	if(BPTpsHUDClass.Class !=nullptr)
	{
		HUDClass = BPTpsHUDClass.Class;
	}
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	PlayerControllerClass = ACoopPlayerController::StaticClass();
	
	PlayerColors.Add(FLinearColor::Blue);
	PlayerColors.Add(FLinearColor::Red);
	PlayerColors.Add(FLinearColor::Yellow);
	PlayerColors.Add(FLinearColor::Green);
	LastPlayerColorIndex = -1;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
	OnActorKilled.AddDynamic(this,&ASGameMode::ActorGetKilled);	
}

void ASGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (FConstPlayerControllerIterator It= GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		if(APlayerController* PC = It->Get(); PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
	PrepareNextWave();
}

void ASGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnActorKilled.RemoveDynamic(this,&ASGameMode::ActorGetKilled);
	GetWorldTimerManager().ClearAllTimersForObject(this);
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
}

void ASGameMode::StartWave()
{
	//TODO : show sign to all players about the count
	WaveCount++;
	NumBotsToSpawn = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(Th_SpawnBots, this,&ASGameMode::SpawnBotTimerElapsed, 1.0f,
																						true,0.0f);
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	//TODO : show sign to all players about stats
	GetWorldTimerManager().ClearTimer(Th_SpawnBots);
	SetWaveState(EWaveState::WaveComplete);
}

void ASGameMode::RestartWave()
{
	GetWorldTimerManager().ClearTimer(Th_SpawnBots);	
}

void ASGameMode::PrepareNextWave()
{
	UE_LOG(LogTemp,Warning,TEXT("GAMEMODE : Preparing next wave"));
	GetWorldTimerManager().SetTimer(Th_NextWaveStart, this,
			&ASGameMode::StartWave, TimeBetweenWaves, false);
	SetWaveState(EWaveState::WaitingToStart);
}

void ASGameMode::CheckWaveState()
{
	if (const bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(Th_NextWaveStart);
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

// ReSharper disable once CppMemberFunctionMayBeConst
void ASGameMode::ActorGetKilled(AActor*	VictimActor, AActor* KillerActor, const AController* VictimController,
                                AController* KillerController)
{
	ASPlayerState* KillerPlayerState = KillerController ? Cast<ASPlayerState>(KillerController->PlayerState) : nullptr;
	if(const ASPlayerState* VictimPlayerState = VictimController ?
		Cast<ASPlayerState>(VictimController->PlayerState) : nullptr;
		KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->AddToScore(100);
		NumBotsToSpawn--;
		if(NumBotsToSpawn<=0)
		{
			PrepareNextWave();
		}
	}
	if(VictimActor->Implements<UIHealthyActor>())
	{
		const auto I = Cast<IIHealthyActor>(VictimActor);
		if(!I->I_GetHealthComp()->IsFriendly(VictimActor,KillerActor))
		{
			if(const auto PController = Cast<ACoopPlayerController>(KillerController);
				IsValid(PController))
			{
				PController->SetHudScore(KillerPlayerState->GetScore());
			}			
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("GAMEMODE : We kill a friend"));
		}
	}
}

void ASGameMode::RestartPlayerDeferred(AController* NewPlayer)
{
	if(!IsAnyPlayerAlive())
	{
		GameOver();
	}
	else
	{
		Super::RestartPlayer(NewPlayer);
	}	
}

void ASGameMode::RespawnPlayer(const APlayerController* PlayerController)
{
	if(!IsAnyPlayerAlive()){GameOver();}
	if(PlayerController)
	{
		FTimerHandle Th_Respawn;
		if(!GetWorldTimerManager().IsTimerActive(Th_Respawn))
		{
			FTimerDelegate TimerDel;
			TimerDel.BindUFunction(this,FName(TEXT("RestartPlayerDeferred")),
															PlayerController);
			GetWorldTimerManager().SetTimer(Th_Respawn,TimerDel,6.f,false);
		}
	}
	else
	{
		GameOver();
	}
}

void ASGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
	UE_LOG(LogTemp,Warning,TEXT("GAME OVER"));
	//TODO : show restart game panel.
}

void ASGameMode::SetWaveState(const EWaveState NewWaveState) const
{
	if (ASGameState* GS = GetGameState<ASGameState>();
		ensureAlways(GS))
	{
		GS->SetWaveState(NewWaveState);
	}
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumBotsToSpawn--;
	if(NumBotsToSpawn <=0)
	{
		EndWave();
	}
	else
	{
		if(!GetWorldTimerManager().IsTimerActive(Th_CheckWaveState))
		{
			GetWorldTimerManager().SetTimer(Th_CheckWaveState,this,&ASGameMode::CheckWaveState,2.0f,true);
		}
	}
}

bool ASGameMode::IsAnyPlayerAlive() const
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		if(const APlayerController* PC = It->Get(); PC)
		{
			if(PC->GetPawn())
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
		if(const IIHealthyActor* I = Cast<IIHealthyActor>(TestPawn);
			IsValid(I->I_GetHealthComp()) && I->I_GetHealthComp()->GetHealth() > 0.0f)
		{
			return true;
		}
	}
	return false;
}