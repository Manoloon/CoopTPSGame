// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/DamageType.h"
#include "SProjectile.generated.h"

class UProjectileMovementComponent;
USTRUCT()
struct FRepPacket 
{
	GENERATED_BODY()

	UPROPERTY()
	float ServerFireTime;
	UPROPERTY()
	bool bCausedDamage;
};

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

UCLASS(Abstract)
class COOPTPS_API ASProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ASProjectile();
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Settings")
		UProjectileMovementComponent* ProjectileComp;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
		FVector InitialLocalVelocity;
	
protected:
	UPROPERTY(ReplicatedUsing = ONREP_RepPacket)
		FRepPacket ReplicationPacket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|DATA")
		FProjectileData Data;

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerExplode();

	virtual void BeginPlay() override;
	virtual void Explode();
	virtual void Destroyed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION()
	void ONREP_RepPacket();
};
