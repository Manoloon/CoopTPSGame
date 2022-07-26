// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SWeapon.h"
#include "HitScanWeapon.generated.h"

USTRUCT()
struct FHitScanTrace 
{
	GENERATED_BODY()

	UPROPERTY()
	float ServerFireTime;
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY()
	bool bCausedDamage;
	UPROPERTY()
	FVector_NetQuantize ImpactPoint;
	UPROPERTY()
	FVector_NetQuantize ImpactNormal;
};

UCLASS(Abstract)
class COOPTPS_API AHitScanWeapon : public ASWeapon
{
	GENERATED_BODY()
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;
	UFUNCTION()
	void OnRep_HitScanTrace() const;
	virtual void HandleFiring() override;
};
