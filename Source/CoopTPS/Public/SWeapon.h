// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class USoundCue;
class UAudioComponent;

UCLASS()
class COOPTPS_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();
	virtual void BeginPlay() override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Weapon")
		TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float BaseDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float ReloadTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		bool bIsReloading;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0,ClampMax=30))
		int32 CurrentAmmo;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 8, ClampMax = 30))
		int32 MaxAmmo;

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
		TSubclassOf<UCameraShake>FireCamShake;

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


	void PlayVFX(FVector TraceEnd);
	virtual void Fire();
	void Reload();
	
public:
	virtual void StartFire();
	virtual void StopFire();
	void StartReloading();	
};
