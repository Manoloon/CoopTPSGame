// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Pawn.h"
#include "STrackerBall.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;
class UAudioComponent;

UCLASS(Abstract)
class COOPTPS_API ASTrackerBall : public APawn, public IIHealthyActor
{
	GENERATED_BODY()
public:
	// Sets default values for this pawn's properties
	ASTrackerBall();

	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual USHealthComponent* I_GetHealthComp() const override;
protected: 
	UPROPERTY(Category = "Componentes", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USHealthComponent* HealthComp = nullptr;
	UPROPERTY(Category = "Componentes", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* SphereComp = nullptr;
	UPROPERTY(Category = "Componentes", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* AudioComp = nullptr;
	UPROPERTY(Category = "Componentes", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp = nullptr;
	
	UPROPERTY(Category = "FX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* ExplosionSFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* FoundTargetSFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* RollSFX = nullptr;
	
	UPROPERTY(Category = "Settings", EditDefaultsOnly)
		float ExplosionRadius = 600.0f;
	UPROPERTY(Category = "Settings", EditDefaultsOnly)
		float ExplosionDamage = 80.0f;
	UPROPERTY(Category = "Settings", EditAnywhere)
		bool bVelocityChanged;
	UPROPERTY(Category = "Settings", EditAnywhere)
		float MovementForce = 7000.0f;
	UPROPERTY(Category = "Settings", EditAnywhere,BlueprintReadOnly)
		float RequireDistanceToTarget = 100.0f;
	UPROPERTY(Category = "Settings", EditAnywhere, BlueprintReadOnly)
		UMaterialInstanceDynamic* MeshMaterialInstance = nullptr;

	FVector NextPathPoint;
	bool bExploded;
	bool bStartedSelfDestruction;
	float SelfDamageInterval = 0.25;
	FTimerHandle TH_HitShake;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	FVector GetNextPathPoint();
	void SelfDamage();
	void SelfDestruct();
	void RefreshPath();
	//TODO NO SE USA
	void StartHitShake();
};
