// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"

#include "Core/CoopPlayerController.h"
#include "CoopTPS.h"
#include "Components/LagCompensationComp.h"
#include "Entities/SCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


FAutoConsoleVariableRef CVarDebugWeaponHitScan(TEXT("COOP.DebugHitScan"),
AHitScanWeapon::DebugWeaponDrawing,TEXT("Draw debug lines for weapon fire"),ECVF_Cheat);

void AHitScanWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHitScanWeapon, HitScanTrace,COND_SkipOwner);
}

void AHitScanWeapon::HandleFiring()
{
	if(const ASCharacter* Pawn = Cast<ASCharacter>(GetOwner()))
	{
		if(CurrentAmmo >=WeaponConfig.BurstCount)
		{
			PlayerController = PlayerController == nullptr ?
				Cast<ACoopPlayerController>(Cast<APawn>(GetOwner())->GetController()) : PlayerController;
			FVector EyeLocation;
			FRotator EyeRotation;
			
			// particle "Target" parameter - la necesitamos por el hitpoint
			EPhysicalSurface SurfaceType = SurfaceType_Default;
			for(int32 i =0;i<WeaponConfig.BurstCount;i++)
			{
				GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
				const float HalfRad = FMath::DegreesToRadians(WeaponConfig.BulletSpread);
				FVector ShotDirection = EyeRotation.Vector();
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad,	HalfRad);
				const FVector TraceEnd = EyeLocation + (ShotDirection * WeaponConfig.MaxBulletTravelDist);
				FVector_NetQuantize TracerEndPoint = TraceEnd;
				PlayShootVfx(TracerEndPoint);
				SpendAmmo();				
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(GetOwner());
				QueryParams.AddIgnoredActor(this);
				QueryParams.bTraceComplex = false;
				QueryParams.bReturnPhysicalMaterial = true;
				FHitResult Hit;
				if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
				{
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				
					PlayImpactFX(SurfaceType,Hit.ImpactPoint,Hit.ImpactNormal);
					TracerEndPoint = Hit.ImpactPoint;
				}
				float ActualDamage = WeaponConfig.BaseDamage;
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 4.0f;
				}
				if (HasAuthority())
				{
					UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection, Hit, 
					GetOwner()->GetInstigatorController(), GetOwner(), WeaponConfig.DamageType);
					HitScanTrace.ImpactPoint = TracerEndPoint;
					HitScanTrace.ImpactNormal =Hit.ImpactNormal;
					HitScanTrace.SurfaceType = SurfaceType;
				}
				if(!HasAuthority() && bUseServerSideRewind && PlayerController)
				{
					if(Pawn && Pawn->GetLagCompensationComp())
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
