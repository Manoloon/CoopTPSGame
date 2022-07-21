// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"

#include "CoopPlayerController.h"
#include "CoopTPS.h"
#include "LagCompensationComp.h"
#include "SCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponHitScan(TEXT("COOP.DebugHitScan"),
DebugWeaponDrawing,TEXT("Draw debug lines for weapon fire"),ECVF_Cheat);

void AHitScanWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHitScanWeapon, HitScanTrace,COND_SkipOwner);
}

void AHitScanWeapon::Fire()
{
	if(GetOwner())
	{
		Super::Fire();
		if(CurrentAmmo >0)
		{
			SpendAmmo();
			FVector EyeLocation;
			FRotator EyeRotation;
			GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			// Bullet Spread - Override shotdirection.
			FVector ShotDirection = EyeRotation.Vector();
			const float HalfRad = FMath::DegreesToRadians(WeaponConfig.BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad,	HalfRad);
			const FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
			// particle "Target" parameter - la necesitamos por el hitpoint
			FVector_NetQuantize TracerEndPoint = TraceEnd;
			EPhysicalSurface SurfaceType = SurfaceType_Default;
			PlayShootVfx(TracerEndPoint);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(GetOwner());
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
 				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				
				PlayImpactFX(SurfaceType,Hit.ImpactPoint,Hit.ImpactNormal);
				TracerEndPoint = Hit.ImpactPoint;
			}
			float ActualDamage = WeaponConfig.BaseDamage;
			if (HasAuthority())
			{
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 4.0f;
				}
				UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection, Hit, 
				GetOwner()->GetInstigatorController(), GetOwner(), WeaponConfig.DamageType);
				HitScanTrace.ImpactPoint = TracerEndPoint;
				HitScanTrace.ImpactNormal =Hit.ImpactNormal;
				HitScanTrace.SurfaceType = SurfaceType;
			}
			if(!HasAuthority() && bUseServerSideRewind && PlayerController)
			{
				if(const ASCharacter* Pawn = Cast<ASCharacter>(GetOwner());
					Pawn && Pawn->GetLagCompensationComp())
				{
					Pawn->GetLagCompensationComp()->
					ServerScoreRequest(Hit.GetActor(),EyeLocation,Hit.Location,
					                   PlayerController->GetServerTime() - PlayerController->HalfRoundtripTime,
					                   this,ActualDamage,WeaponConfig.DamageType);
				}
			}
			if(DebugWeaponDrawing>0)
			{
				DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green,false, 1.0f);
				DrawDebugSphere(GetWorld(),TracerEndPoint,10.0f,12,FColor::Emerald,false,
																1.f,0,2);
			}
			LastFireTime = GetWorld()->TimeSeconds;
		}
	}
}

void AHitScanWeapon::OnRep_HitScanTrace() const
{
	if(!HitScanTrace.ImpactNormal.IsZero())
	{
		PlayShootVfx(HitScanTrace.ImpactPoint);
		PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.ImpactPoint,HitScanTrace.ImpactNormal);
	}
}
