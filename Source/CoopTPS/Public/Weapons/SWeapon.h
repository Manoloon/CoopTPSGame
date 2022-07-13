// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "ST_WeaponData.h"
#include "UI/ST_HUDData.h"
#include "SWeapon.generated.h"

class ACoopPlayerController;
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
	virtual void StartFire();
	virtual void StopFire();
	void StartReloading();
	void EquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket) const;
	void DropWeapon();
	void FinishReloading();
	void SetInitialInfoUI();
	const FHUDData& GetCrosshairData() const;
	UAnimMontage* GetFireMontage() const;
	UAnimMontage* GetReloadMontage() const;
	FTransform GetWeaponHandle()const;
	FName GetWeaponName() const;
	bool HaveAmmoInMag()const;
	int32 GetWeaponCurrentAmmo() const;
	int32 GetWeaponMaxAmmo()const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool bIsReloading;
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		int32 CurrentAmmo=WeaponConfig.MaxAmmo;
	int32 CurrentAmmoInBackpack=CurrentAmmo;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* WeaponAudioComponent;

	UPROPERTY()
	ACoopPlayerController* PlayerController;
	bool bIsEquipped=false;
	float LastFireTime;
	float TimeBetweenShots;
	FTimerHandle Th_TimeBetweenShots;

	void PlayShootVfx(FVector TraceEnd) const;
	virtual void Fire();
	virtual void OnRep_Owner() override;
	void Reload();
	void UpdateAmmoInfoUI();
	UFUNCTION(Server,Reliable,WithValidation)
		virtual void ServerReload();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerFire();
	UFUNCTION(Server, Reliable)
		void ServerEquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket) const;
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

	void CalculateAmmo();
	void SetPickable() const;
};
