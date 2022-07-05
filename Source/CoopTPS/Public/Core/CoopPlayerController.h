// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IInputComm.h"
#include "CoopPlayerController.generated.h"

class ASCharacter;

UCLASS()
class COOPTPS_API ACoopPlayerController : public APlayerController
{
	GENERATED_BODY()
	UPROPERTY()
	class ATPSHud* PlayerHUD;
	
public:
	ACoopPlayerController()=default;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerID;
	UPROPERTY(BlueprintReadWrite)
	ASCharacter* MyPawn = nullptr;

	void SetHudScore(const int Score);
	void SetHudGameTime();
	//void SetHUDHealth(const float Health);

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
protected:
	virtual void SetupInputComponent() override;
	IIInputComm* PawnInterface;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* APawn) override;
	virtual void OnUnPossess() override;
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
	void StartFiring();
	void StopFiring();
	void Reload();
	void ChangeWeapon();
	void StartThrow();
	void StopThrow();
	void Jump();

	/* Sync time between client and server*/
	float MatchTime =120.f;
	uint32 Countdown=0;
 	float ClientServerDelta = 0.f; //Diff between client and server time
	UPROPERTY(EditAnywhere,Category = Settings)
	float TimeSyncFrequency = 5.f;
	UFUNCTION(Server,Reliable)
	void Server_RequestServerTime(float TimeClientRequest);
	UFUNCTION(Client,Reliable)
	void Client_ReportServerTime(float TimeClientRequest,float TimeServerReceiveClientRequest);
	UFUNCTION()
	void SyncTime();
	
};
