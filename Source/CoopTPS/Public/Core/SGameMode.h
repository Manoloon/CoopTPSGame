// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnActorKilled, AActor*,VictimActor, AActor*, KillerActor,
									const AController*, VictimController ,AController*, KillerController);

UCLASS()
class COOPTPS_API ASGameMode final : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASGameMode();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void RespawnPlayer(const APlayerController* PlayerController);
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;
	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
	UPROPERTY(BlueprintReadWrite)
	TArray<FLinearColor>PlayerColors;
	UPROPERTY(BlueprintReadWrite)
	int32 LastPlayerColorIndex;

private:
	FTimerHandle Th_SpawnBots;
	FTimerHandle Th_NextWaveStart;
	FTimerHandle Th_CheckWaveState;
	int32 NumBotsToSpawn;
	int32 WaveCount;
	// Helper function
	bool IsAnyPlayerAlive() const;
	bool IsAnyEnemyAlive() const;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float TimeBetweenWaves;
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void SpawnNewBot();
	
	void SpawnBotTimerElapsed();
	void StartWave();
	void EndWave();
	void RestartWave();
	void PrepareNextWave();
	void CheckWaveState();
	UFUNCTION()
	void ActorGetKilled(AActor*	VictimActor, AActor* KillerActor, const AController* VictimController,
																AController* KillerController);
	UFUNCTION()
	void RestartPlayerDeferred(AController* NewPlayer);
	void GameOver();
	void SetWaveState(EWaveState NewWaveState) const;

//	void CalculateMatchCountdown();
//	void CalculateGameTime();	
};
