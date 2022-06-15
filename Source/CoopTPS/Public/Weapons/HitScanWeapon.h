// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SWeapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class COOPTPS_API AHitScanWeapon : public ASWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire() override;
private:
	UPROPERTY(ReplicatedUsing = ONREP_HitScanTrace)
		FHitScanTrace HitScanTrace;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	UFUNCTION()
	void ONREP_HitScanTrace();
};
