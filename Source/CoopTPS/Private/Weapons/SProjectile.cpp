// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugProjectile(TEXT("COOP.Projectile"),
DebugWeaponDrawing,TEXT("Draw debug Projectile Explosion Radius"),ECVF_Cheat);
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
	MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	RootComponent = MeshComp;
}

void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileComp->SetVelocityInLocalSpace(InitialLocalVelocity);
	FTimerHandle ExplodeTH;
	GetWorldTimerManager().SetTimer(ExplodeTH, this, &ASProjectile::Explode,
									Data.ExplodeDelay, false, Data.ExplodeDelay);
}

void ASProjectile::Explode()
{
	if(DebugWeaponDrawing)
	{
		DrawDebugSphere(GetWorld(),GetActorLocation(),Data.DamageRadius,12,
						FColor::Orange,true,133.0f,0,12.f);
	}
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerExplode();
	}
	if(Data.ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Data.ExplosionSFX, GetActorLocation());
	}
	if(Data.DefaultExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Data.DefaultExplosionFX, GetActorLocation());
	}
	if(GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		// TODO : add team players
		IgnoredActors.Add(this);
		IgnoredActors.Add(GetInstigatorController());
		UGameplayStatics::ApplyRadialDamage(this, Data.MaxDamage,
			GetActorLocation(),Data.DamageRadius,Data.DamageType, IgnoredActors,
			this, GetInstigatorController(),true);
		SetLifeSpan(1.0f);
	}
}

void ASProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ASProjectile::ServerExplode_Implementation()
{
	Explode();
}

bool ASProjectile::ServerExplode_Validate()
{
	return true;
}
