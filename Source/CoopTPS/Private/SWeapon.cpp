// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Components/MeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopTPS.h"
#include "TimerManager.h"
#include "Camera/CameraShake.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"

// console debuging
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"),DebugWeaponDrawing,TEXT("Draw debug lines for weapon fire"),ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{
 	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
	BaseDamage = 20.0f;
	FireRate = 600;
	ReloadTime = 3.0f;
	CurrentAmmo = 30;
	MaxAmmo = 30;
	BulletSpread = 2.0f;
	bReplicates = true;

	/* hace que el update de la red sea mas rapido, NO LAG*/
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60/ FireRate; // 10 balas por segundo;
}

void ASWeapon::Fire()
{
	// DE ESTA MANERA SOLO Corre todo el FIRE si es ROLE_AUTHORITY
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}
	AActor* MyOwner = GetOwner();
	if (MyOwner && CurrentAmmo > 0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		// Bullet Spread - Override shotdirection.
		FVector ShotDirection = EyeRotation.Vector();
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// particle "Target" parameter - la necesitamos por el hitpoint
		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		FHitResult Hit;

		// Trace VFX
		PlayVFX(TracerEndPoint);
		//SFX
		if (FireSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSFX, EyeLocation);
		}

		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
			
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
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green, false, 1.0f);
		}
		
	}
	LastFireTime = GetWorld()->TimeSeconds;
	CurrentAmmo -= 1;
}

/*ACTIONS*/

void ASWeapon::Reload()
{
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
}

void ASWeapon::StartReloading()
{
	if (CurrentAmmo < MaxAmmo) 
	{
	bIsReloading = true;
	GetWorldTimerManager().SetTimer(TimeHandle_Reloading, this, &ASWeapon::Reload,ReloadTime);
	}
}

void ASWeapon::StartFire()
{
	if (CurrentAmmo>1)
	{
		float FireDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f); // busca un numero entre lo primero y cero , nunca dara negativo.
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBeetwenShots, this, &ASWeapon::Fire, TimeBetweenShots, true,FireDelay);
	}
	else
	{
		if(!bIsReloading)
		{
			StartReloading();
		}		
	}
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBeetwenShots);
}

/*Play Impact FX*/

void ASWeapon::PlayImpactFX(EPhysicalSurface NewSurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedFX = nullptr;
	switch (NewSurfaceType)
	{
	case SURFACE_FLESHDEFAULT: //flesh default
	case SURFACE_FLESHVULNERABLE: // flesh headshot
		SelectedFX = FleshImpactFX;
		break;

	default:
		SelectedFX = DefaultImpactFX;
		break;
	}
	if (SelectedFX)
	{
		FVector MuzzleLoc = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLoc;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, ImpactPoint, ShotDirection.Rotation());
	}
}

/*Play Trace VFX*/

void ASWeapon::PlayVFX(FVector TraceEnd)
{
	if (MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);
	}
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, MuzzleLocation);
	if (TracerComp)
	{
		TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
	}
	// Camera shake
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if(MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if(PC)
		{
			PC->ClientStartCameraShake(FireCamShake);
		}
	}
}


// NETWORK FIRE ACTION

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ONREP_HitScanTrace()
{
	//Play cosmetic FX
	PlayVFX(HitScanTrace.TraceTo);
	PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}