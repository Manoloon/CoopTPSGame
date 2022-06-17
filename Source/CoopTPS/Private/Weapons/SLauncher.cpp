// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SLauncher.h"

#include "UnrealNetwork.h"
#include "Weapons/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void ASLauncher::StartFire()
{
	if(!GetWorldTimerManager().IsTimerActive(ChargingProjectileTH))
	{
		GetWorldTimerManager().SetTimer(ChargingProjectileTH,this,&ASLauncher::ServerUpdateThrow,
			GetWorld()->GetDeltaSeconds(),true);
	}
}

void ASLauncher::StopFire()
{
	GetWorldTimerManager().ClearTimer(ChargingProjectileTH);
	ServerLaunchProjectile();
}

void ASLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

bool ASLauncher::ServerLaunchProjectile_Validate()
{
	return true;
}

void ASLauncher::ServerLaunchProjectile_Implementation()
{
	LaunchDistance = 100;
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	UE_LOG(LogTemp,Warning,TEXT("Server InitialLocalVelocity = %f"),LauncherPacket.InitialVelocity.X);
	if (ASProjectile* LocalProjectile = GetWorld()->SpawnActorDeferred<ASProjectile>
		(ProjectileClass, FTransform(GetOwner()->GetActorRotation(), MuzzleLocation)))
	{
		LocalProjectile->SetInstigator(GetInstigator());
		LocalProjectile->SetOwner(GetOwner());
		LocalProjectile->InitialLocalVelocity = LauncherPacket.InitialVelocity;
		UE_LOG(LogTemp,Warning,TEXT("INSIDE InitialLocalVelocity = %f"),LauncherPacket.InitialVelocity.X);
		UGameplayStatics::FinishSpawningActor(LocalProjectile, FTransform(GetOwner()->GetActorRotation(),
			MuzzleLocation, FVector(1.0)));
	}
	//SFX
	if (WeaponFXConfig.FireSFX)
	{
		const FVector_NetQuantize loc = GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFXConfig.FireSFX,loc);
	}
	PlayShootVFX(MuzzleLocation);
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
	UE_LOG(LogTemp,Warning,TEXT("InitialLocalVelocity = %f"),LauncherPacket.InitialVelocity.X);
}

void ASLauncher::OnRep_LauncherPacket()
{
	//const FVector_NetQuantize MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	//PlayShootVFX(MuzzleLocation);	
}
void ASLauncher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASLauncher,LauncherPacket,COND_OwnerOnly);
}
