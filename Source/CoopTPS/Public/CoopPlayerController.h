// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IInputComm.h"
#include "CoopPlayerController.generated.h"

/**
 * 
 */
class ASCharacter;

UCLASS()
class COOPTPS_API ACoopPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerID;
	UPROPERTY(BlueprintReadWrite)
	ASCharacter* MyPawn = nullptr;

	virtual void SetupInputComponent() override;
protected:
	IIInputComm* PawnInterface;
	virtual void OnPossess(APawn* APawn) override;
	// inputs
	void StartRun();
	void StopRun();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnRate(float Value);
	void LookUpRate(float Value);
	void StartAds();
	void StopAds();
	void StartCrouch();
	void StopCrouch();
	void StartFire();
	void StopFire();
	void Reload();
	void ChangeWeapon();
	void StartThrow();
	void StopThrow();
	void Jump();
	
};
