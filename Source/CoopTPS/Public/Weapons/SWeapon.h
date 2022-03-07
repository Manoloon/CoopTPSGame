// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UMatineeCameraShake;
class USoundCue;
class UAudioComponent;

USTRUCT()
struct FWeaponData
{
	GENERATED_BODY()

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
	// Sets default values for this actor's properties
	ASWeapon();
	virtual void BeginPlay() override;

protected:

	void PlayImpactFX(EPhysicalSurface NewSurfaceType, FVector ImpactPoint);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Weapon")
		const TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float BaseDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float ReloadTime;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		bool bIsReloading;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0,ClampMax=30))
		int32 CurrentAmmo;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 8, ClampMax = 30))
		int32 MaxAmmo;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0, ClampMax = 13))
		float BulletSpread;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

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
		TSubclassOf<UMatineeCameraShake>FireCamShake;

	// SFX
	UPROPERTY(EditDefaultsOnly,Category = "Weapon")
		USoundCue* ReloadSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		USoundCue* NoAmmoSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		USoundCue* FireSFX;

		UAudioComponent* WeaponAudioComponent;

	// fire time handlers
	FTimerHandle TimerHandle_TimeBeetwenShots;
	float LastFireTime;
	//deriva de FireRate
	float TimeBetweenShots;

	// reload Time Handlers
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

public:
	virtual void StartFire();
	virtual void StopFire();
	void StartReloading();	
};
