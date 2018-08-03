// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBall.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;
class UAudioComponent;

UCLASS()
class COOPTPS_API ASTrackerBall : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBall();

	UPROPERTY(Category = MeshComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComp;

	FVector NextPathPoint;

	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly)
		bool bVelocityChanged;
	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly)
		float MovementForce;
	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly,BlueprintReadOnly)
		float RequireDistanceToTarget;
	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly, BlueprintReadOnly)
		UMaterialInstanceDynamic* MatInst;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FVector GetNextPathPoint();
	UPROPERTY(Category = HealthComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USHealthComponent* HealthComp;
	UPROPERTY(Category = HealthComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* SphereComp;
	UPROPERTY(Category = AudioComp, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* AudioComp;
	UPROPERTY(Category = "FX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ExplosionFX;
	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly)
		float ExplosionRadius;
	UPROPERTY(Category = "TrackerBall", EditDefaultsOnly)
		float ExplosionDamage;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* ExplosionSFX;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* FoundTargetSFX;
	UPROPERTY(Category = "SFX", EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
		USoundCue* RollSFX;

	bool bExploded;
	bool bStartedSelfDestruction;
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	void SelfDamage();

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	void SelfDestruct();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
