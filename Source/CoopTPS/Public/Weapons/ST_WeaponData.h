#pragma once
#include "Sound/SoundCue.h"
#include "GameFramework/DamageType.h"
#include "ST_WeaponData.generated.h"

USTRUCT()
struct FHitScanTrace 
{
	GENERATED_BODY()
	
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	FVector_NetQuantize TraceTo;
};

USTRUCT(BlueprintType)
struct FWeaponFXData
{
	GENERATED_BODY()
	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerFX;	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase>FireCamShake;

	// SFX
	UPROPERTY(EditDefaultsOnly,Category = "Weapon")
	USoundCue* ReloadSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* NoAmmoSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* FireSFX;

	FWeaponFXData()
	{
		MuzzleFX=nullptr;
		DefaultImpactFX=nullptr;
		FleshImpactFX=nullptr;
		TracerFX=nullptr;
		FireCamShake=nullptr;
		ReloadSFX=nullptr;
		NoAmmoSFX=nullptr;
		FireSFX=nullptr;
	}
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 1, ClampMax = 30))
	int32 MaxAmmo;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0, ClampMax = 13))
	float BulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadTime;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName MuzzleSocketName;
	UPROPERTY(EditDefaultsOnly,Category ="Weapon")
	TSubclassOf<UDamageType> DamageType;

	FWeaponData()
	{
		MaxAmmo = 30;
		BulletSpread = 2.0f;
		BaseDamage = 20.0f;
		FireRate = 600;
		ReloadTime = 3.0f;
		MuzzleSocketName = "MuzzleSocket";
	}
};