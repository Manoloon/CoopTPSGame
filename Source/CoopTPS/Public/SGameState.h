// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"


UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	WaitingToComplete,

	WaveComplete,

	GameOver,
};

UCLASS()
class COOPTPS_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	UFUNCTION()
		void OnRep_WaveState(EWaveState OldWaveState);

	UPROPERTY(BlueprintReadOnly, Replicatedusing = ONREP_WaveState, Category = "WaveState")
		EWaveState WaveState;

	UFUNCTION(BlueprintImplementableEvent, Category = "WaveState")
		void WaveStateChanged(EWaveState NewWaveState, EWaveState OldWaveState);
public:

	UFUNCTION()
		void SetWaveState(EWaveState NewWaveState);

};
