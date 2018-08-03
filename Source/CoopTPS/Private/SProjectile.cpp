// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileComp->SetVelocityInLocalSpace(InitialLocalVelocity);
}
