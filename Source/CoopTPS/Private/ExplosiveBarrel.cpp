// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosiveBarrel.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "SHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "PhysicsEngine/RadialForceComponent.h"


// debug
static int32 DebugBarrelExp = 0;
FAutoConsoleVariableRef CVARDebugBarrelExp(
	TEXT("Coop.DebugBarrelExp"),
	DebugBarrelExp,
	TEXT("draw debug lines for Explosion Barrel"),
	ECVF_Cheat);

// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;

	ExplosionDamage = 40.0f;
	ExplosionRadius = 400.0f;
	ExplosionImpulse = 400.0f;
	bExploded = false;
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
}

void AExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
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
	FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;	
	MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

	MeshComp->SetMaterial(0, ExplodedMaterial);

	// push all physics radial force
	RadialForceComp->FireImpulse();
	

	if (GetLocalRole()== ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugBarrelExp)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 3.0f);
		}
	}
}

