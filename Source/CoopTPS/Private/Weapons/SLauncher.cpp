// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SLauncher.h"
#include "Weapons/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void ASLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASLauncher::StartFire()
{
	if(!GetWorldTimerManager().IsTimerActive(ChargingProjectileTH))
	{
		GetWorldTimerManager().SetTimer(ChargingProjectileTH,this,&ASLauncher::UpdateThrow,
			GetWorld()->GetDeltaSeconds(),true);
	}
}

void ASLauncher::StopFire()
{
	GetWorldTimerManager().ClearTimer(ChargingProjectileTH);
	FinishLaunch();
}

void ASLauncher::UpdateThrow()
{
	LaunchDistance = FMath::Clamp<float>(LaunchDistance + 10.0f, 1.0f, MaxLaunchDistance);
	InitialLocalVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance);
}

void ASLauncher::LaunchProjectile() const
{
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	if (ASProjectile* LocalProjectile = GetWorld()->SpawnActorDeferred<ASProjectile>
		(ProjectileClass, FTransform(GetOwner()->GetActorRotation(), MuzzleLocation)))
	{
		LocalProjectile->InitialLocalVelocity = InitialLocalVelocity;
		UGameplayStatics::FinishSpawningActor(LocalProjectile, FTransform(GetOwner()->GetActorRotation(),
			MuzzleLocation, FVector(1.0)));
	}
}

void ASLauncher::FinishLaunch()
{
	LaunchDistance = 100;
	LaunchProjectile();
}

void ASLauncher::Fire()
{
 	Super::Fire();
 	if(GetOwner() && ProjectileClass)
 	{
 		UpdateThrow();
	}
}
