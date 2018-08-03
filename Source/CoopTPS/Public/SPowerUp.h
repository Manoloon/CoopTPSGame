// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerUp.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
UCLASS()
class COOPTPS_API ASPowerUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUp();

protected:
	USceneComponent * SceneComp;
	UPROPERTY(Category = MeshComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp;
	UPROPERTY(Category = LightComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPointLightComponent* LightComp;


	UPROPERTY(EditAnywhere)
	float PowerUpInterval;
	UPROPERTY(EditAnywhere)
	int32 TotalNumTicks;

	int32 TicksProcess;
	FTimerHandle TimerHandle_PowerUpTicks;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnTickPowerUp();

public:
	void ActivatePowerUp();

	UFUNCTION(BlueprintImplementableEvent)
		void OnActivated();
	UFUNCTION(BlueprintImplementableEvent)
		void OnExpired();
	UFUNCTION(BlueprintImplementableEvent)
		void OnPowerUpTicked();
};
