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

UCLASS()
class COOPTPS_API AExplosiveBarrel final : public AActor, public IIHealthyActor
{
	GENERATED_BODY()
	
public:	
	AExplosiveBarrel();
	virtual USHealthComponent* I_GetHealthComp() const override;
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(Category = MeshComp, VisibleAnywhere)
		UStaticMeshComponent* MeshComp;
	UPROPERTY(Category = Components, VisibleAnywhere)
		USHealthComponent* HealthComp;
	UPROPERTY(Category = Components, VisibleAnywhere)
		URadialForceComponent* RadialForceComp;
	UPROPERTY(Category = Components, VisibleAnywhere)
		UAudioComponent* AudioComp;
	
	UPROPERTY(Category = "FX", EditDefaultsOnly)
		UParticleSystem* ExplosionFX;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionRadius;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionDamage;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionImpulse;
	UPROPERTY(Category = "SFX", EditDefaultsOnly)
		USoundCue* ExplosionSFX;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* HitSFX;
	UPROPERTY(Category = "MatInst", EditDefaultsOnly)
		UMaterialInstanceDynamic* MatInst;
	UPROPERTY(Category = "MatInst", EditDefaultsOnly)
		UMaterial* ExplodedMaterial;

private:
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	bool bExploded;
	void SelfDestruct();
};
