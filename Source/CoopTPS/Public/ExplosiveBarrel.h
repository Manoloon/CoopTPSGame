// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"
class USHealthComponent;
class URadialForceComponent;
class UAudioComponent;
class UParticleSystem;
class USoundCue;
class UStaticMeshComponent;

UCLASS()
class COOPTPS_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Category = HealthComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USHealthComponent* HealthComp;
	UPROPERTY(Category = RadialForceComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		URadialForceComponent* RadialForceComp;
	UPROPERTY(Category = AudioComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* AudioComp;
	UPROPERTY(Category = "FX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionFX;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionRadius;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionDamage;
	UPROPERTY(Category = "Explosion", EditDefaultsOnly)
		float ExplosionImpulse;

	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* ExplosionSFX;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* HitSFX;
	UPROPERTY(Category = "MatInst", EditDefaultsOnly, BlueprintReadOnly)
		UMaterialInstanceDynamic* MatInst;
	UPROPERTY(Category = "MatInst", EditDefaultsOnly, BlueprintReadOnly)
		UMaterial* ExplodedMaterial;
	UPROPERTY(Category = MeshComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp;

	bool bExploded;

protected:

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void SelfDestruct();
	
};
