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

ASWeapon::ASWeapon()
{
 	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	// Ojo con esto que nos estamos cargando la posibilidad de ajuste de epic.
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	bReplicates =true;
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
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}
	if (GetOwner() && CurrentAmmo > 0)
	{
		//SFX
		if (WeaponFXConfig.FireSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFXConfig.FireSFX,GetActorLocation());
		}
	}
	else
	{
		StopFire();
		// auto reload.
		StartReloading();
	}
	LastFireTime = GetWorld()->TimeSeconds;
	CurrentAmmo -= 1;
}

void ASWeapon::Reload()
{
	if(WeaponFXConfig.ReloadSFX)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFXConfig.ReloadSFX);
	}
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

const FHUDData& ASWeapon::GetCrosshairData() const
{
	return CrossHairData;
}

void ASWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ASWeapon::StartFire()
{
	if (CurrentAmmo>1 && !GetWorldTimerManager().IsTimerActive(TimeBetweenShotsTH))
	{
		const float FireDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->GetTimeSeconds(),0.0f); 
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
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, ImpactPoint,
																	ShotDirection.Rotation());
	}
}

void ASWeapon::PlayShootVFX(const FVector TraceEnd) const
{
	if (WeaponFXConfig.MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(WeaponFXConfig.MuzzleFX, MeshComp, WeaponConfig.MuzzleSocketName);
	}
	const FVector_NetQuantize MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
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
//call on client | execute on server 
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
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

void ASWeapon::ONREP_HitScanTrace()
{
	//Play cosmetic FX
	PlayShootVFX(HitScanTrace.ImpactPoint);
	PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.ImpactPoint);
}
// this is not declare on the header. Dont need to.
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}