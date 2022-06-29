// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/ExplosiveBarrel.h"
#include "Particles/ParticleSystem.h"
#include "Components/SHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicsEngine/RadialForceComponent.h"

static bool DebugBarrel = false;
FAutoConsoleVariableRef CVarDebugBarrelExp(TEXT("Coop.DebugBarrelExp"),
	DebugBarrel,TEXT("draw debug lines for Explosion Barrel"),ECVF_Cheat);

AExplosiveBarrel::AExplosiveBarrel()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates=true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->TeamNum = 2;
	
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->ImpulseStrength = ExplosionImpulse;
}

USHealthComponent* AExplosiveBarrel::I_GetHealthComp() const
{
	if(HealthComp)
	{
		return HealthComp;
	}
	return nullptr;	
}

void AExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;
	bExploded = false;
}
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	if(HealthComp && HasAuthority())
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::HealthChanged);
	}
}

void AExplosiveBarrel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	HealthComp->OnHealthChanged.RemoveDynamic(this,&AExplosiveBarrel::HealthChanged);
}

void AExplosiveBarrel::HealthChanged(USHealthComponent* OwningHealthComp, const float Health,float HealthDelta,
                                     const class UDamageType* DamageType, class AController* InstigatedBy,AActor* DamageCauser)
{
	UE_LOG(LogTemp,Warning,TEXT("TAKING DAMAGE"));
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
	OnRep_Exploded();
	if(ExplosionFX && ExplosionSFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}
	
	// push the barrel upward! 
	const FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;	
	MeshComp->AddImpulse(BoostIntensity, NAME_None, true);
	MeshComp->SetMaterial(0, ExplodedMaterial);
	RadialForceComp->FireImpulse();

	if (GetLocalRole()== ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage,GetActorLocation(),
			ExplosionRadius, nullptr, IgnoredActors,this, GetInstigatorController(),
																							true);

		if (DebugBarrel)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12,
									FColor::Red, false, 2.0f, 3.0f);
		}
	}
}

void AExplosiveBarrel::OnRep_Exploded() const
{
	if(ExplosionFX && ExplosionSFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}
}

void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExplosiveBarrel,bExploded);
}
