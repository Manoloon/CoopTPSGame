// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Public/DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Components/MeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
}

void ASWeapon::Fire()
{
	FHitResult Hit;
	AActor* MyOwner = GetOwner();
	if(MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + ( ShotDirection * 10000);

		// particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(Hit,EyeLocation,TraceEnd,ECC_Visibility,QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			if (ImpactFX) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			TracerEndPoint = Hit.ImpactPoint;
		}
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green, false, 1.0f);
		
		if (MuzzleFX)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);
		}
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



