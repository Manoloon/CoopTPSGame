// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class COOPTPS_API ASProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASProjectile();
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category ="Granada")
	UProjectileMovementComponent* ProjectileComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granada")
		FVector InitialLocalVelocity;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
