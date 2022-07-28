#pragma once
#include "Sound/SoundCue.h"
#include "GameFramework/DamageType.h"
#include "ST_WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponFXData
{
	GENERATED_BODY()
	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	UParticleSystem* MuzzleFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	UParticleSystem* DefaultImpactFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	UParticleSystem* FleshImpactFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	UParticleSystem* TracerFX;	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase>FireCamShake;

	// SFX
	UPROPERTY(EditDefaultsOnly,Category = "Weapon|SFX")
	USoundCue* ReloadSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|SFX")
	USoundCue* NoAmmoSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|SFX")
	USoundCue* FireSFX;

	// Montage
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	UAnimMontage* FireMontage;
	
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
		ReloadMontage=nullptr;
		FireMontage=nullptr;
	}
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 1, ClampMax = 30))
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere)
	int32 MaxAmmoInBackpack;
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 0, ClampMax = 13))
	float BulletSpread;
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 1, ClampMax = 12))
	int32 BurstCount;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 MaxBulletTravelDist;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireRate;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float ReloadTime;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName MuzzleSocketName;
	UPROPERTY(EditAnywhere,Category ="Weapon")
	TSubclassOf<UDamageType> DamageType;

	FWeaponData()
	{
		MaxAmmo = 30;
		MaxAmmoInBackpack=60;
		BulletSpread = 2.0f;
		BurstCount=1;
		MaxBulletTravelDist=8000;
		BaseDamage = 20.0f;
		FireRate = 600;
		ReloadTime = 3.0f;
		WeaponName = "Rifle";
		MuzzleSocketName = "MuzzleSocket";
	}
};