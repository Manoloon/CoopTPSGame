// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SLauncher.h"
#include "Weapons/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"

void ASLauncher::Fire()
{
 	Super::Fire();
 
 	if(GetOwner() && ProjectileClass)
 	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		const FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// spawn the projectile at the muzzle
 		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, FRotator::ZeroRotator,
 																								ActorSpawnParams);
	}
}
