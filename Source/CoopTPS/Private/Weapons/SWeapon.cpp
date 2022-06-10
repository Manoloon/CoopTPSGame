// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopTPS.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

// console debuging
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"),DebugWeaponDrawing,TEXT("Draw debug lines for weapon fire"),ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
 	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	bReplicates = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	/* hace que el update de la red sea mas rapido, NO LAG*/
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		SphereComp->OnComponentBeginOverlap.AddDynamic(this,&ASWeapon::OnSphereOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this,&ASWeapon::OnSphereEndOverlap);
	}
	if(WeaponConfig.FireRate >0)
	{
		TimeBetweenShots = 60/ WeaponConfig.FireRate; // 10 balas por segundo;
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("FireRate is less or equal To ZERO"));
	}
}

void ASWeapon::Fire()
{
	// DE ESTA MANERA SOLO Corre FIRE si es ROLE_AUTHORITY
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}
	if (GetOwner() && CurrentAmmo > 0)
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
		FHitResult Hit;
		
		PlayVFX(TracerEndPoint);
		//SFX
		if (WeaponFXConfig.FireSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFXConfig.FireSFX, EyeLocation);
		}

		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
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
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green, false, 1.0f);
		}
		
	}
	else
	{
		StopFire();
		StartReloading();
	}
	LastFireTime = GetWorld()->TimeSeconds;
	CurrentAmmo -= 1;
}

void ASWeapon::Reload()
{
	CurrentAmmo = WeaponConfig.MaxAmmo;
	bIsReloading = false;
}

void ASWeapon::StartReloading()
{
	if (CurrentAmmo < WeaponConfig.MaxAmmo && !bIsReloading) 
	{
	bIsReloading = true;
	GetWorldTimerManager().SetTimer(ReloadingTH, this, &ASWeapon::Reload,WeaponConfig.ReloadTime);
	}
}

void ASWeapon::StartFire()
{
	if (CurrentAmmo>1 && !GetWorldTimerManager().IsTimerActive(TimeBetweenShotsTH))
	{
		// busca un numero entre lo primero y cero , nunca dara negativo.
		const float FireDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f); 
		GetWorldTimerManager().SetTimer(TimeBetweenShotsTH, this, &ASWeapon::Fire,
														TimeBetweenShots, true,FireDelay);
	}
	else if(!bIsReloading)
	{
		StartReloading();
	}
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimeBetweenShotsTH);
}

void ASWeapon::PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector ImpactPoint) const
{
	UParticleSystem* SelectedFX;
	switch (NewSurfaceType)
	{
	case SURFACE_FLESHDEFAULT: //flesh default
	case SURFACE_FLESHVULNERABLE: // flesh headshot
		SelectedFX = WeaponFXConfig.FleshImpactFX;
		break;

	default:
		SelectedFX = WeaponFXConfig.DefaultImpactFX;
		break;
	}
	if (SelectedFX)
	{
		const FVector MuzzleLoc = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLoc;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::PlayVFX(const FVector TraceEnd)
{
	if (WeaponFXConfig.MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(WeaponFXConfig.MuzzleFX, MeshComp, WeaponConfig.MuzzleSocketName);
	}
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	if (UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
																			WeaponFXConfig.TracerFX, MuzzleLocation))
	{
		TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
	}
	// Camera shake
	if(const APawn* MyOwner = Cast<APawn>(GetOwner()))
	{
		if(APlayerController* PC = Cast<APlayerController>(MyOwner->GetController()))
		{
			PC->ClientStartCameraShake(WeaponFXConfig.FireCamShake);
		}
	}
}

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

void ASWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// show widget to the player whos overlapping,
	// OtherActor->ShowWidget(this) -> this function will show the proper widget
	//only for the player who is overlapping.
}

void ASWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// OtherActor->ShowWidget(null)
}

void ASWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}
