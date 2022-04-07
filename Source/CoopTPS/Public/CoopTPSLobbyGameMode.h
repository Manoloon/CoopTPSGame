// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopTPSLobbyGameMode.generated.h"

UCLASS()
class COOPTPS_API ACoopTPSLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

		ACoopTPSLobbyGameMode();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:

	UPROPERTY(BlueprintReadWrite)
		TArray<FLinearColor>PlayerColors;
	UPROPERTY(BlueprintReadWrite)
		int32 LastPlayerColorIndex;

	uint32 NumOfPlayers = 0;
	void TravelToMap();	

private:
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;
};
