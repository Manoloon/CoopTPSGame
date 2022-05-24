// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickup.generated.h"
class USphereComponent;
class DecalComponent;
class ASPowerUp;
UCLASS()
class COOPTPS_API ASPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickup();
protected:
	UPROPERTY(Category = SphereComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* SphereComp;
	UPROPERTY(Category = DecalComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly,Category = "PowerUP")
	TSubclassOf<ASPowerUp>PowerupClass;

	ASPowerUp* PowerupInstance;
	UPROPERTY(EditAnywhere, Category = "PowerUP")
	float CooldDownDuraction;

	FTimerHandle TimerHandle_RespawnTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RespawnPowerUp();

public:	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
