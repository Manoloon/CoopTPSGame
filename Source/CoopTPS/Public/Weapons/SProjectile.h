// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/DamageType.h"
#include "SProjectile.generated.h"

class UProjectileMovementComponent;

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		float DamageRadius;
	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		float MaxDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		float ExplodeDelay;
	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		USoundCue* ExplosionSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Default DATA")
		UParticleSystem* DefaultExplosionFX;
	//defaults
	FProjectileData()
	{
		DamageRadius = 200.0f;
		MaxDamage = 1000.0f;
		ExplodeDelay = 2.0;
		DamageType = UDamageType::StaticClass();
		DefaultExplosionFX =nullptr;
		ExplosionSFX = nullptr;
	}
};

UCLASS()
class COOPTPS_API ASProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASProjectile();
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Default Settings")
	UProjectileMovementComponent* ProjectileComp;

	UPROPERTY(EditDefaultsOnly, Category = "Default Settings")
		class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default Settings")
		FVector InitialLocalVelocity;

protected:
	virtual void Explode();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default DATA")
		FProjectileData Data;

	FTimerHandle TimerHandler_Explode;

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerExplode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
