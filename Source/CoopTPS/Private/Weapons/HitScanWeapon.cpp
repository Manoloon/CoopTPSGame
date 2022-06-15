// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"
#include "CoopTPS.h"
#include "PhysicalMaterial.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// console debuging
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponHitScan(TEXT("COOP.DebugHitScan"),
DebugWeaponDrawing,TEXT("Draw debug lines for weapon fire"),ECVF_Cheat);

void AHitScanWeapon::Fire()
{
	Super::Fire();
	if(GetOwner() && CurrentAmmo >0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		// Bullet Spread - Override shotdirection.
		FVector ShotDirection = EyeRotation.Vector();
		float HalfRad = FMath::DegreesToRadians(WeaponConfig.BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;
		// particle "Target" parameter - la necesitamos por el hitpoint
		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		PlayVFX(TracerEndPoint);
		if (FHitResult Hit; GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd,
																	COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = WeaponConfig.BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
			GetOwner()->GetInstigatorController(), GetOwner(), WeaponConfig.DamageType);
			
			PlayImpactFX(SurfaceType,Hit.ImpactPoint);
			TracerEndPoint = Hit.ImpactPoint;			
		}
		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
		if(DebugWeaponDrawing>0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green,
																false, 1.0f);
		}
	}
}

void AHitScanWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHitScanWeapon, HitScanTrace,COND_SkipOwner);
}

void AHitScanWeapon::ONREP_HitScanTrace()
{
	//Play cosmetic FX
	PlayVFX(HitScanTrace.TraceTo);
	PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}
