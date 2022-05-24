// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*,
	VictimActor, AActor*, KillerActor, AController*, KillerController);

UCLASS()
class COOPTPS_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FLinearColor>PlayerColors;
	UPROPERTY(BlueprintReadWrite)
	int32 LastPlayerColorIndex;

protected:
	FTimerHandle TimerHandle_SpawnBots;
	FTimerHandle TH_NextWaveStart;
	int32 NumBotsToSpawn;
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float TimeBetweenWaves;
	
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void SpawnNewBot();
	
	void SpawnBotTimerElapsed();
	
	void StartWave();

	void EndWave();
	
	void PrepareNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewWaveState);

	void RestoreDeadPlayer();

public:
	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaTime) override;

	
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
		FOnActorKilled OnActorKilled;
};
