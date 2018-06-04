// Fill out your copyright notice in the Description page of Project Settings.

#include "SLauncher.h"
#include "SProjectile.h"
#include "Engine/World.h"


void ASLauncher::Fire()
{
	AActor* MyOwner = GetOwner();

	if(MyOwner && ProjectileClass)
	{
		
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// aca le digo que el instigador soy yo.
		//ActorSpawnParams.Instigator = MyOwner;
		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
	}
}
