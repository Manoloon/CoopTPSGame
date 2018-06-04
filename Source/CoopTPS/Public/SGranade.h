// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGranade.generated.h"

class ASProjectile;
/**
 * 
 */
UCLASS()
class COOPTPS_API ASGranade : public ASWeapon
{
	GENERATED_BODY()
	
	
protected :
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
		TSubclassOf<ASProjectile>ProjectileClass;
	UPROPERTY(VisibleDefaultsOnly,Category = "Projectil")
	float LaunchDistance;
	float PathLifeTime = 5.0f;
	float TimeInterval = 0.05f;
	
	FVector StartLocation;
	FVector InitialVelocity;
	FVector InitialLocalVelocity;
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectil")
	FVector Gravity;


	virtual void Fire() override;

	virtual void StartFire() override;
	virtual void StopFire() override;
};
