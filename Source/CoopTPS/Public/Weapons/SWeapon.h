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
	void EquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket);
	void DropWeapon();
	void FinishReloading();
	const FHUDData& GetCrosshairData() const;
	UAnimMontage* GetFireMontage() const;
	UAnimMontage* GetReloadMontage() const;
	FTransform GetWeaponHandle()const;
	FName GetWeaponName() const;
	bool HaveAmmoInMag()const;
	int32 GetWeaponCurrentAmmo() const;
	FName GetWeaponsName() const;
	int32 GetAmmoInBackpack() const;
	int32 GetWeaponMaxAmmo()const;

protected:
	UPROPERTY() //ReplicatedUsing=OnRep_Reloading
		bool bIsReloading;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FHUDData CrossHairData;
	UPROPERTY(EditAnywhere,Category = "Settings")
		FWeaponData WeaponConfig;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		FWeaponFXData WeaponFXConfig;

	UPROPERTY(EditAnywhere,Category = "Settings|Network")
	bool bUseServerSideRewind = false;
	
	void PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector& ImpactPoint, const FVector& ImpactNormal) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere,Category = "Component")
		USphereComponent* SphereComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		int32 CurrentAmmo=WeaponConfig.MaxAmmo;
	UPROPERTY(Replicated)
		int32 CurrentAmmoInBackpack=CurrentAmmo;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName TracerTargetName = "BeamEnd";
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* WeaponAudioComponent;

	UPROPERTY()
	ACoopPlayerController* PlayerController;
	UPROPERTY()
	APawn* PlayerPawn;
	bool bIsEquipped=false;
	float LastFireTime;
	float TimeBetweenShots;
	FTimerHandle Th_TimeBetweenShots;
	// its use for Ammo data reconciliation
	int32 AmmoSequence=0;
	void SpendAmmo();
	void PlayShootVfx(FVector TraceEnd) const;
	virtual void Fire();
	void Reload();
	void UpdateAmmoInfoUI();
	virtual void OnRep_Owner() override;
	//UFUNCTION()
	//void OnRep_Reloading();
	UFUNCTION()
	void OnRep_CurrentAmmo();
	UFUNCTION(Server,Reliable,WithValidation)
		virtual void ServerReload();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerFire();
	UFUNCTION(Server, Reliable)
		void ServerEquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket);
	UFUNCTION(Server,Reliable)
		void ServerDropWeapon();
	UFUNCTION(Client,Reliable)
		void ClientAmmoUpdate(int32 ServerAmo);
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
