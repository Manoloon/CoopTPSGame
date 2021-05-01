// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SLauncher.generated.h"

class ASProjectile;

/**
 * 
 */
UCLASS()
class COOPTPS_API ASLauncher : public ASWeapon
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
		TSubclassOf<ASProjectile>ProjectileClass;
	
public : 
	void Fire() override;
};
