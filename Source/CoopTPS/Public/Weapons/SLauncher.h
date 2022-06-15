// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SLauncher.generated.h"

class ASProjectile;

/* Projectile weapon */
UCLASS(Abstract)
class COOPTPS_API ASLauncher : public ASWeapon
{
	GENERATED_BODY()

protected:
	FVector InitialLocalVelocity;
	FVector InitialVelocity;
	FTimerHandle ChargingProjectileTH;
	const FVector GRAVITY = FVector(0.0f,0.0f,-900.0f);
	bool bChargingMode = false;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	float LaunchDistance = 100.0f;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	float MaxLaunchDistance = 1000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<ASProjectile>ProjectileClass;
	virtual void Tick(float DeltaSeconds) override;
	virtual void StartFire() override;
	virtual void StopFire() override;
	void UpdateThrow();
	void LaunchProjectile() const;
	void FinishLaunch();
public : 
	virtual void Fire() override;
};
