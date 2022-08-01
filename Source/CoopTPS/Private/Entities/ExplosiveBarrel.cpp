// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/ExplosiveBarrel.h"
#include "Particles/ParticleSystem.h"
#include "Components/SHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
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
	PrimaryActorTick.bStartWithTickEnabled = false;
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
	RadialForceComp->SetActive(false);
	RadialForceComp->PrimaryComponentTick.bStartWithTickEnabled=false;
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->ImpulseStrength = ExplosionImpulse;
}

void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExplosiveBarrel,bExploded);
	DOREPLIFETIME(AExplosiveBarrel, ExplosionFX);
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
	//SetReplicateMovement(true);
	if(HealthComp && HasAuthority())
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::HealthChanged);
	}
}

void AExplosiveBarrel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(HealthComp && HasAuthority())
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this,&AExplosiveBarrel::HealthChanged);
	}
}

void AExplosiveBarrel::AfterExplode()
{
	SetReplicateMovement(false);
	RadialForceComp->SetComponentTickEnabled(false);
	RadialForceComp->Activate(false);
	MeshComp->SetCollisionObjectType(ECC_WorldStatic);
	MeshComp->SetSimulatePhysics(false);
}

void AExplosiveBarrel::HealthChanged(USHealthComponent* OwningHealthComp, const float Health,float HealthDelta,
                                     const class UDamageType* DamageType, class AController* InstigatedBy,AActor* DamageCauser)
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
	OnRep_Exploded();
	if(HasAuthority())
	{
		if(ExplosionFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		}
		if(ExplosionSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
		}
	}
	/*
	if(ExplosionFX && ExplosionSFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}
	*/
	// push the barrel upward!
	SetReplicateMovement(true);
	const FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;	
	MeshComp->AddImpulse(BoostIntensity, NAME_None, true);
	MeshComp->SetMaterial(0, ExplodedMaterial);
	RadialForceComp->Activate(true);
	RadialForceComp->SetComponentTickEnabled(true);
	RadialForceComp->FireImpulse();

	if (HasAuthority())
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
	FTimerHandle Th_AfterExplode;
	GetWorldTimerManager().SetTimer(Th_AfterExplode,this,&AExplosiveBarrel::AfterExplode,3.0f);
}

void AExplosiveBarrel::OnRep_Exploded() const
{
	if(ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}
	if(ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}
}

