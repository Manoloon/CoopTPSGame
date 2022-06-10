// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/ExplosiveBarrel.h"
#include "Particles/ParticleSystem.h"
#include "Components/SHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicsEngine/RadialForceComponent.h"

// debug
static int32 DebugBarrelExp = 0;
FAutoConsoleVariableRef CVARDebugBarrelExp(
	TEXT("Coop.DebugBarrelExp"),
	DebugBarrelExp,
	TEXT("draw debug lines for Explosion Barrel"),
	ECVF_Cheat);

AExplosiveBarrel::AExplosiveBarrel()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
}

USHealthComponent* AExplosiveBarrel::I_GetHealthComp() const
{
	return HealthComp;
}

void AExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;

	ExplosionDamage = 40.0f;
	ExplosionRadius = 400.0f;
	ExplosionImpulse = 400.0f;
	bExploded = false;
}

void AExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health,
			float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy,
																		AActor* DamageCauser)
{
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,
															MeshComp->GetMaterial(0));
	}
	if(MatInst)
	{
		MatInst->SetScalarParameterValue("DamageTaken", GetWorld()->TimeSeconds);
	}
	if(Health<=0.0f)
	{
		SelfDestruct();
	}
}

void AExplosiveBarrel::SelfDestruct()
{
	if (bExploded) { return; }

	bExploded = true;
	const UWorld* World = GetWorld();
	UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionFX, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	
	// push the barrel upward! 
	const FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;	
	MeshComp->AddImpulse(BoostIntensity, NAME_None, true);
	MeshComp->SetMaterial(0, ExplodedMaterial);
	RadialForceComp->FireImpulse();
	

	if (GetLocalRole()== ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage,
			GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors,
							this, GetInstigatorController(), true);

		if (DebugBarrelExp)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12,
									FColor::Red, false, 2.0f, 3.0f);
		}
	}
}

