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
	ASTrackerBall();
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual USHealthComponent* I_GetHealthComp() const override;
protected: 
	UPROPERTY(Category = "Componentes", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USHealthComponent> HealthComp = nullptr;
	UPROPERTY(Category = "Componentes", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USphereComponent> SphereComp = nullptr;
	UPROPERTY(Category = "Componentes", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UAudioComponent> AudioComp = nullptr;
	UPROPERTY(Category = "Componentes", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UStaticMeshComponent> MeshComp = nullptr;
	
	UPROPERTY(Category = "FX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UParticleSystem> ExplosionFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USoundBase> ExplosionSFX = nullptr;
	UPROPERTY(Category = "FX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UParticleSystem> SelfExplosionFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USoundBase> SelfExplosionSFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USoundBase> FoundTargetSFX = nullptr;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USoundBase> RollSFX = nullptr;
	
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
	UPROPERTY(Category = "Settings", EditAnywhere,BlueprintReadOnly)
		TSubclassOf<UDamageType> LocDamageType;
	UPROPERTY(Category = "Settings", EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UMaterialInstanceDynamic> MeshMaterialInstance = nullptr;

	FVector NextPathPoint;
	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;
	bool bStartedSelfDestruction;
	float SelfDamageInterval = 0.25;
	UPROPERTY(Replicated)
	TObjectPtr<UParticleSystem> CurrentExplosionFX = nullptr;
	UPROPERTY(Replicated)
	TObjectPtr<USoundBase> CurrentExplosionSFX=nullptr;
	FTimerHandle Th_HitShake;
		
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
		void HealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	UFUNCTION()
	void OnRep_Exploded() const;
	void CalculateMovement();
	FVector GetNextPathPoint();
	void SelfDamage();
	void SelfDestruct(AController* DamageInstigator);
	void RefreshPath();
	//TODO NO SE USA
	void StartHitShake();
};
