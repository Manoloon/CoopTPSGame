// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameModePVP.generated.h"

UCLASS()
class AGameModePvp final : public AGameMode
{
	GENERATED_BODY()
	void SetAllPlayersEnemies();
};
