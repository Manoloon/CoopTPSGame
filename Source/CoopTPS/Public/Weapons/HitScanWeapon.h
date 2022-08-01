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
class COOPTPS_API AHitScanWeapon final : public ASWeapon
{
	GENERATED_BODY()
protected:
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;
	//void TraceWithScatter(const FVector& HitTarget,const TArray<FVector>& HitTargets);
	UFUNCTION()
	void OnRep_HitScanTrace() const;
	virtual void HandleFiring() override;
public:
	inline static int32 DebugWeaponDrawing = 0;
};
