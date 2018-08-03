// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

UCLASS()
class COOPTPS_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
protected:
	// timer for spawn bots
	FTimerHandle TimerHandle_SpawnBots;
	FTimerHandle TimerHandle_NextWaveStart;

	// number of bots to spawn in wave
	int32 NumBotsToSpawn;

	// number of the wave
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float TimeBetweenWaves;
	
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void SpawnNewBot();

	// esto lleva el calculo de cuando arrancar una nueva wave.
	void SpawnBotTimerElapsed();
	
	void StartWave();

	void EndWave();
	
	void PrepareNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewWaveState);

public:
	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaTime) override;
};
