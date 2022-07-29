// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickup.generated.h"
class USphereComponent;
class DecalComponent;
class ASPowerUp;
UCLASS()
class COOPTPS_API ASPickup final : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPickup();
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
protected:
	UPROPERTY(Category = SphereComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* SphereComp;
	UPROPERTY(Category = DecalComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly,Category = "Settings")
	TSubclassOf<ASPowerUp>PowerUpClass;

	UPROPERTY()
	ASPowerUp* PowerUpInstance;
	UPROPERTY(EditAnywhere, Category = "Settings")
	float CoolDownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	virtual void BeginPlay() override;
	void RespawnPowerUp();
};
