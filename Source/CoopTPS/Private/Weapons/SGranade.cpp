// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SGranade.h"
#include "Weapons/SProjectile.h"
#include "Engine/World.h"

void ASGranade::HandleFiring()
{
	if (GetOwner() && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		const FVector WeaponLocation = GetActorLocation();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// aca le digo que el instigador soy yo.
		//ActorSpawnParams.Instigator = MyOwner;
		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, WeaponLocation, EyeRotation, ActorSpawnParams);
	}
}

void ASGranade::StartFire()
{
	if (GetOwner())
	{
		//StartLocation = GetActorLocation() + FRotator::RotateVector((FVector(100.0f,0.0f,-10.0f)),MyOwner->GetControlRotation())
	}
}

void ASGranade::StopFire()
{
	HandleFiring();
}
