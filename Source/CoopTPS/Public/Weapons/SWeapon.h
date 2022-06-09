// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "ChaosEngineInterface.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UMatineeCameraShake;
class USoundCue;
class UAudioComponent;

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 8, ClampMax = 30))
	int32 MaxAmmo;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0, ClampMax = 13))
	float BulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadTime;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

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

// Contains one Hitscan of the weapon Single trace.
USTRUCT()
struct FHitScanTrace 
{
GENERATED_BODY()

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPTPS_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		bool bIsReloading;

	virtual void StartFire();
	virtual void StopFire();
	void StartReloading();	
protected:
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponData WeaponConfig;
	void PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector ImpactPoint) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Weapon")
		const TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0,ClampMax=30))
		int32 CurrentAmmo=30;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
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
	UPROPERTY()
		UAudioComponent* WeaponAudioComponent;

	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;
	float TimeBetweenShots;
	FTimerHandle TimeHandle_Reloading;

	//EPhysicalSurface SurfaceType;

	void PlayVFX(FVector TraceEnd);
	virtual void Fire();
	void Reload();
	
	// newtworking se crea una implementation en cpp. 
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	UPROPERTY(ReplicatedUsing = ONREP_HitScanTrace)
		FHitScanTrace HitScanTrace;

	UFUNCTION()
		void ONREP_HitScanTrace();
};
