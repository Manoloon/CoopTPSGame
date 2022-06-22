// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

class UMaterial;
class UMaterialInstanceDynamic;
class USHealthComponent;
class URadialForceComponent;
class UAudioComponent;
class UParticleSystem;
class USoundCue;
class UStaticMeshComponent;

UCLASS(Abstract)
class COOPTPS_API AExplosiveBarrel : public AActor, public IIHealthyActor
{
	GENERATED_BODY()
	
public:	
	AExplosiveBarrel();
	virtual USHealthComponent* I_GetHealthComp() const override;
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere,Category = "Components")
		UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere,Category = "Components")
		USHealthComponent* HealthComp;
	UPROPERTY(VisibleAnywhere,Category = "Components")
		URadialForceComponent* RadialForceComp;
	UPROPERTY(VisibleAnywhere,Category = "Components")
		UAudioComponent* AudioComp;
	
	UPROPERTY(EditAnywhere,Category = "Settings")
		float ExplosionRadius=300.f;
	UPROPERTY(EditAnywhere,Category = "Settings")
		float ExplosionDamage=60.f;
	UPROPERTY(EditAnywhere,Category = "Settings")
		float ExplosionImpulse=400.f;
	UPROPERTY(EditAnywhere,Category = "Settings")
		UParticleSystem* ExplosionFX;
	UPROPERTY(EditDefaultsOnly,Category = "Settings|SFX")
		USoundCue* ExplosionSFX;
	UPROPERTY(EditDefaultsOnly,Category = "Settings|SFX")
		USoundCue* HitSFX;
	UPROPERTY()//UPROPERTY(EditDefaultsOnly,Category = "Settings")
		UMaterialInstanceDynamic* MatInst;
	UPROPERTY(EditDefaultsOnly,Category = "Settings")
		UMaterial* ExplodedMaterial;

private:
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	bool bExploded;
	void SelfDestruct();
};
