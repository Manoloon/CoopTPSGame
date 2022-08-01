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
	// Local + server
	virtual void StartFire();
	virtual void StopFire();
	void StartReload();
	void EquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket);
	void DropWeapon();
	void FinishReloading();
	const FHUDData& GetCrosshairData() const;
	UAnimMontage* GetFireMontage() const;
	UAnimMontage* GetReloadMontage() const;
	FTransform GetWeaponHandle()const;
	FName GetWeaponName() const;
	bool HaveAmmoInMag()const;
	int32 GetCurrentAmmo() const;
	FName GetWeaponsName() const;
	int32 GetAmmoInBackpack() const;
	int32 GetWeaponMaxAmmo()const;

protected:
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FHUDData CrossHairData;
	UPROPERTY(EditAnywhere,Category = "Settings")
		FWeaponData WeaponConfig;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponFXData WeaponFXConfig;
	UPROPERTY(EditAnywhere,Category = "Settings|Network")
		bool bUseServerSideRewind = false;
	UPROPERTY(Transient,ReplicatedUsing=OnRep_CurrentAmmo)
		int32 CurrentAmmo;
	UPROPERTY(Transient,ReplicatedUsing=OnRep_AmmoInBackpack)
		int32 CurrentAmmoInBackpack;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
	UPROPERTY()
	ACoopPlayerController* PlayerController;
	FTimerHandle Th_FinishReload;
	bool bIsEquipped=false;
	float LastFireTime;
	float LastReloadTime;
	float TimeBetweenShots;
	FTimerHandle Th_TimeBetweenShots;
	// its use for Ammo data reconciliation
	int32 AmmoSequence=0;
//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere,Category = "Component")
		USphereComponent* SphereComp;
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* WeaponAudioComponent;
	
//networking
	UFUNCTION()
		void OnRep_CurrentAmmo();
	UFUNCTION()
		void OnRep_AmmoInBackpack();
	virtual void OnRep_Owner() override;
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_StartReload();
	UFUNCTION(Server, Reliable)
		void ServerEquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket);
	UFUNCTION(Client,Reliable)
		void ClientAmmoUpdate(const bool bCalculateAmmoSeq,int32 ServerAmmo);
	UFUNCTION(Client,Reliable)
		void ClientAmmoReload(int32 AmmoToAdd);
	
////////////////////////////////////////////////////////////////////////////////////	
//functions	
	void SpendAmmo();
	virtual void HandleFiring(){};
	void UpdateAmmoInfoUI();
	void PlayAudioFX(USoundCue* SfxToPlay) const;
	void PlayShootVfx(FVector TraceEnd) const;
	void PlayImpactFX(const EPhysicalSurface NewSurfaceType,
						const FVector& ImpactPoint, const FVector& ImpactNormal) const;

	
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

	void CalculateAmmoReloaded();
	void SetPickable();
};
