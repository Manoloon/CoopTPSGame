// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

UCLASS()
class COOPTPS_API ASPlayerState final : public APlayerState
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable,Category = "Score" )
		void AddScore(const float ScoreDelta);
	
	
};
