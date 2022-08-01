// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SLauncher.h"

#include "Net/UnrealNetwork.h"
#include "Weapons/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void ASLauncher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASLauncher,LauncherPacket,COND_OwnerOnly);
}

void ASLauncher::StartFire()
{
	if(CurrentAmmo <=0 || IsReloading()){return;}
	
	if(!GetWorldTimerManager().IsTimerActive(Th_ChargingProjectile) && CurrentAmmo>0)
	{
		SpendAmmo();
		GetWorldTimerManager().SetTimer(Th_ChargingProjectile,this,&ASLauncher::ServerUpdateThrow,
			GetWorld()->GetDeltaSeconds(),true);
	}
}

void ASLauncher::StopFire()
{
	if(GetWorldTimerManager().IsTimerActive(Th_ChargingProjectile))
	{
		GetWorldTimerManager().ClearTimer(Th_ChargingProjectile);
		ServerLaunchProjectile();
	}
}

bool ASLauncher::ServerLaunchProjectile_Validate()
{
	return true;
}

void ASLauncher::ServerLaunchProjectile_Implementation()
{
	LaunchDistance = 100;
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	if (ASProjectile* LocalProjectile = GetWorld()->SpawnActorDeferred<ASProjectile>
		(ProjectileClass, FTransform(GetOwner()->GetActorRotation(), MuzzleLocation)))
	{
		LocalProjectile->SetInstigator(GetInstigator());
		LocalProjectile->SetOwner(GetOwner());
		LocalProjectile->InitialLocalVelocity = LauncherPacket.InitialVelocity;
		UGameplayStatics::FinishSpawningActor(LocalProjectile, FTransform(GetOwner()->GetActorRotation(),
			MuzzleLocation, FVector(1.0)));
	}
	if (WeaponFXConfig.FireSFX)
	{
		const FVector_NetQuantize Loc = GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFXConfig.FireSFX,Loc);
	}
	PlayShootVfx(MuzzleLocation);
}

void ASLauncher::ServerUpdateThrow_Implementation()
{
	UpdateThrow();
}

bool ASLauncher::ServerUpdateThrow_Validate()
{
	return true;
}

void ASLauncher::UpdateThrow()
{
	LaunchDistance = FMath::Clamp<float>(LaunchDistance + 10.0f, 1.0f, MaxLaunchDistance);
	LauncherPacket.InitialVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance);
}

void ASLauncher::OnRep_LauncherPacket() const
{
	const FVector_NetQuantize MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	PlayShootVfx(MuzzleLocation);	
}
