// Fill out your copyright notice in the Description page of Project Settings.

#include "SLauncher.h"
#include "SProjectile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


void ASLauncher::Fire()
{
 	Super::Fire();
 	AActor* MyOwner = GetOwner();

 	if(MyOwner && ProjectileClass)
 	{
	
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		const FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// spawn the projectile at the muzzle
 		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, FRotator::ZeroRotator, ActorSpawnParams);
	}
}
