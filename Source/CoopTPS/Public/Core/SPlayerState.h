// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

UCLASS()
class COOPTPS_API ASPlayerState final : public APlayerState
{
	GENERATED_BODY()
	UPROPERTY()
	class ASCharacter* PlayerCharacter;
	UPROPERTY()
	class ACoopPlayerController* PlayerController;
public:
	virtual void OnRep_Score() override;
	UFUNCTION(BlueprintCallable)
		void AddToScore(const float ScoreDelta);
};
