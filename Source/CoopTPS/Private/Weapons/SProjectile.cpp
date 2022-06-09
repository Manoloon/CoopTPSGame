// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileComp->SetVelocityInLocalSpace(InitialLocalVelocity);

	GetWorldTimerManager().SetTimer(TimerHandler_Explode, this, &ASProjectile::Explode, Data.ExplodeDelay, false, Data.ExplodeDelay);
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
		UGameplayStatics::ApplyRadialDamage(this, Data.MaxDamage, GetActorLocation(), Data.DamageRadius,
			nullptr, IgnoredActors, this, GetInstigatorController(), true);
	}
}

//network

void ASProjectile::ServerExplode_Implementation()
{
	Explode();
}

bool ASProjectile::ServerExplode_Validate()
{
	return true;
}
