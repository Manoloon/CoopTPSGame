// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"
#include "CoopTPS.h"
#include "PhysicalMaterial.h"
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
		const float HalfRad = FMath::DegreesToRadians(WeaponConfig.BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad,
																	HalfRad);
		const FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		// particle "Target" parameter - la necesitamos por el hitpoint
		FVector_NetQuantize TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		PlayShootVFX(TracerEndPoint);
		if(TraceResult.GetActor())
		{
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(
															TraceResult.PhysMaterial.Get());

			float ActualDamage = WeaponConfig.BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			UGameplayStatics::ApplyPointDamage(TraceResult.GetActor(), ActualDamage, ShotDirection, 
			TraceResult,
			GetOwner()->GetInstigatorController(), GetOwner(), WeaponConfig.DamageType);
			
			PlayImpactFX(SurfaceType,TraceResult.ImpactPoint);
			TracerEndPoint = TraceResult.ImpactPoint;	
		}
		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.ImpactPoint = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
		if(DebugWeaponDrawing>0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green,
																false, 1.0f);
		}
	}
}