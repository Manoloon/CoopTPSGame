// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "ST_WeaponData.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UMatineeCameraShake;
class USoundCue;
class UAudioComponent;

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
private:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(ReplicatedUsing = ONREP_HitScanTrace)
		FHitScanTrace HitScanTrace;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponData WeaponConfig;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponFXData WeaponFXConfig;
	
	void PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector ImpactPoint) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere,Category = "Component")
		USphereComponent* SphereComp;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0,ClampMax=30))
		int32 CurrentAmmo=30;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* WeaponAudioComponent;

	float LastFireTime;
	float TimeBetweenShots;
	FTimerHandle TimeBetweenShotsTH;
	FTimerHandle ReloadingTH;

	void PlayShootVFX(FVector TraceEnd) const;
	virtual void Fire();
	void Reload();
	
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();
	UFUNCTION()
		void ONREP_HitScanTrace();
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);		
};
