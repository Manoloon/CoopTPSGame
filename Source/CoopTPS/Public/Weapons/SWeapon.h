// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "ChaosEngineInterface.h"
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
	//virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
protected:
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

	void PlayVFX(FVector TraceEnd) const;
	virtual void Fire();
	void Reload();
	
	// newtworking se crea una implementation en cpp. 
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();
	
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
