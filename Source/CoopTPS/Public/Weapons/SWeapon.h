// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "ST_WeaponData.h"
#include "UI/ST_HUDData.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UMatineeCameraShake;
class USoundCue;
class UAudioComponent;

UCLASS(Abstract)
class COOPTPS_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	ASWeapon();
	bool IsReloading() const;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool bIsReloading;
	virtual void StartFire();
	virtual void StopFire();
	void StartReloading();
	const FHUDData& GetCrosshairData() const;
	UAnimMontage* GetFireMontage() const;
	UAnimMontage* GetReloadMontage() const;
	FTransform GetWeaponHandle()const;
protected:
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FHUDData CrossHairData;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponData WeaponConfig;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponFXData WeaponFXConfig;
	
	void PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector& ImpactPoint, const FVector& ImpactNormal) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere,Category = "Component")
		USphereComponent* SphereComp;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0,ClampMax=30))
		int32 CurrentAmmo=30;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* WeaponAudioComponent;
	

	float LastFireTime;
	float TimeBetweenShots;
	FTimerHandle Th_TimeBetweenShots;
	FTimerHandle Th_Reloading;

	void PlayShootVfx(FVector TraceEnd) const;
	virtual void Fire();
	void Reload();

	UFUNCTION(Server,Reliable,WithValidation)
		virtual void ServerReload();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerFire();
	
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
