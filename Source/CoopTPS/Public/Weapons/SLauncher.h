// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SLauncher.generated.h"

class ASProjectile;
USTRUCT()
struct FLauncherPacket
{
	GENERATED_BODY()

	UPROPERTY()
	float ServerFireTime;
	UPROPERTY()
	bool bCausedDamage;
	UPROPERTY()
	FVector_NetQuantize InitialVelocity;
	UPROPERTY()
	FVector_NetQuantize ImpactNormal;
};
/* Projectile weapon */
UCLASS(Abstract)
class COOPTPS_API ASLauncher final : public ASWeapon
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing=OnRep_LauncherPacket)
	FLauncherPacket LauncherPacket;
	FTimerHandle Th_ChargingProjectile;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	float LaunchDistance = 100.0f;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	float MaxLaunchDistance = 1000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<ASProjectile>ProjectileClass;
	virtual void StartFire() override;
	virtual void StopFire() override;
	virtual void Tick(float DeltaSeconds) override;
	UFUNCTION()
	void OnRep_LauncherPacket() const;
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerLaunchProjectile();
	/* NO CREO QUE NECESITEMOS UN RPC para esto -> habra que ver como actualizar la variable
	 * initialVelocity en el cliente con el ultimo dato.
	 */
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerUpdateThrow();
	void UpdateThrow();
};
