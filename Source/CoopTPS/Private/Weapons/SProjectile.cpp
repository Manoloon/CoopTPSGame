// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASProjectile::ASProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed = 1500.0f;
	ProjectileComp->MaxSpeed = 1500.0f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->bShouldBounce = true;
	ProjectileComp->Bounciness = 0.2;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComp->SetCollisionObjectType(ECC_WorldDynamic);

	RootComponent = MeshComp;
}

void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileComp->SetVelocityInLocalSpace(InitialLocalVelocity);

	GetWorldTimerManager().SetTimer(ExplodeTH, this, &ASProjectile::Explode, Data.ExplodeDelay, false, Data.ExplodeDelay);
}


void ASProjectile::Explode()
{
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerExplode();
	}
	if(Data.ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Data.ExplosionSFX, GetActorLocation());
	}
	if(GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		// TODO : add team players
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, Data.MaxDamage, GetActorLocation(),
			Data.DamageRadius,nullptr, IgnoredActors, this, GetInstigatorController(),
																								true);
	}
}

void ASProjectile::ServerExplode_Implementation()
{
	Explode();
}

bool ASProjectile::ServerExplode_Validate()
{
	return true;
}
