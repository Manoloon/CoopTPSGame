// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Public/DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Components/MeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopTPS.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

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
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60/ FireRate; // 10 balas por segundo;
}

void ASWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if(MyOwner && CurrentAmmo>0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + ( ShotDirection * 10000);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// particle "Target" parameter - la necesitamos por el hitpoint
		FVector TracerEndPoint = TraceEnd;
		FHitResult Hit;
		
		//SFX
		if (FireSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSFX, EyeLocation);
		}
		
		if (GetWorld()->LineTraceSingleByChannel(Hit,EyeLocation,TraceEnd,COLLISION_WEAPON,QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			UParticleSystem* SelectedFX = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT: //flesh default
				SelectedFX = FleshImpactFX;
				break;
			case SURFACE_FLESHVULNERABLE: // flesh headshot
				SelectedFX = FleshImpactFX;
				break;

			default:
				SelectedFX = DefaultImpactFX;
				break;
			}
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			TracerEndPoint = Hit.ImpactPoint;
			PlayVFX(TracerEndPoint);
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);				

			if (SelectedFX) 
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}

		}
		if(DebugWeaponDrawing>0)
		{
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green, false, 1.0f);
		}
		
	}
	LastFireTime = GetWorld()->TimeSeconds;
	CurrentAmmo -= 1;
}

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
		GLog->Log("RELOAD RELOAD!"); // TODO 
	}
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBeetwenShots);
}

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
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}