// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/STrackerBall.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Components/SHealthComponent.h"
#include "EngineUtils.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "Entities/SCharacter.h"
#include "Sound/SoundCue.h"
#include "components/AudioComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

static int32 DebugTrackballDraw = 0;
FAutoConsoleVariableRef CVarDebugTrackballDraw(
	TEXT("Coop.DebugTrackball"),
	DebugTrackballDraw,
	TEXT("draw debug lines for trackball"),
	ECVF_Cheat);

ASTrackerBall::ASTrackerBall()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealhComp"));
	HealthComp->TeamNum = 255;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBall::OnHealthChanged);
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetSound(RollSFX);
	AudioComp->SetupAttachment(RootComponent);

	bVelocityChanged = false;
	bExploded = false;
	bStartedSelfDestruction = false;
}

void ASTrackerBall::BeginPlay()
{
	Super::BeginPlay();	
	if(HasAuthority())
	{
		NextPathPoint = GetNextPathPoint();
	}
}

void ASTrackerBall::CalculateMovement()
{
	if (HasAuthority() && !bExploded)
	{
		const float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();
		if (GetVelocity().Size() > 10)
		{
			AudioComp->SetSound(RollSFX);
		}
		if (DistanceToTarget <= RequireDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bVelocityChanged);
		}

		if (DebugTrackballDraw)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 10, 12, FColor::Yellow,
			                false, 1.0f, 0, 3.0f);
		}
	}
}

void ASTrackerBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateMovement();
}

FVector ASTrackerBall::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (TActorIterator<APawn>PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		APawn* TestPawn = *PawnIterator;
		if (TestPawn  == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}
		if(const IIHealthyActor* I = Cast<IIHealthyActor>(TestPawn);I->I_GetHealthComp()->GetHealth() > 0.0f)
		{
			if(const float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
				Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if(BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(
							this, GetActorLocation(), BestTarget);
		if (!NavPath) { return GetActorLocation(); }
		/*unstuck method*/
		FTimerHandle TH_RefreshPath;
		GetWorldTimerManager().ClearTimer(TH_RefreshPath);
		GetWorldTimerManager().SetTimer(TH_RefreshPath, this, &ASTrackerBall::RefreshPath,
																		5.0f, false);
		
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			return NavPath->PathPoints[1];
		}
	}	
		return GetActorLocation();	
}

void ASTrackerBall::SelfDamage()
{
	UGameplayStatics::ApplyDamage(this, 10.0f, GetInstigatorController(),
														this, nullptr);
}

void ASTrackerBall::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
 	if(MeshMaterialInstance == nullptr)
 	{
 	MeshMaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,
 																	MeshComp->GetMaterial(0));
 	}
 	if(MeshMaterialInstance)
 	{
 	MeshMaterialInstance->SetScalarParameterValue("DamageTaken",GetWorld()->TimeSeconds);
 	}
	if(!GetWorldTimerManager().IsTimerActive(TH_HitShake))
	{
		GetWorldTimerManager().SetTimer(TH_HitShake,this, &ASTrackerBall::StartHitShake, 0.0f, false);
	}

	if(Health <=0.0f)
	{
		SelfDestruct();
	}
}

void ASTrackerBall::SelfDestruct()
{
	if (bExploded) { return; }
	bExploded = true;
	if(ExplosionFX && ExplosionSFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	}	
	MeshComp->SetVisibility(false, true); 
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(HasAuthority())
	{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(),
		ExplosionRadius, LocDamageType, IgnoredActors, this,GetInstigatorController(),
																			true);

		if(DebugTrackballDraw)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12,
								FColor::Red, false, 2.0f, 3.0f);
		}
	SetLifeSpan(2.0f);
	}
}

void ASTrackerBall::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

void ASTrackerBall::StartHitShake()
{
	if (MeshMaterialInstance == nullptr)
	{
		MeshMaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,
																	MeshComp->GetMaterial(0));
	}
	if (MeshMaterialInstance)
	{
		const float DamageHit = FMath::FInterpTo(1.0f, 0.0, GetWorld()->TimeSeconds, 1.0f);
		MeshMaterialInstance->SetScalarParameterValue("DamageTaken", DamageHit);
		if(DamageHit == 0.0f)
		{
			GetWorldTimerManager().ClearTimer(TH_HitShake);
		}
	}
}

void ASTrackerBall::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if(!bStartedSelfDestruction)
	{
		if (!USHealthComponent::IsFriendly(OtherActor,this))
		{
			// we overlapped player
			if (GetLocalRole() == ROLE_Authority)
			{
				FTimerHandle TH_SelfDamage;
				GetWorldTimerManager().SetTimer(TH_SelfDamage, this, &ASTrackerBall::SelfDamage,
											SelfDamageInterval, true, 0.0f);
				bStartedSelfDestruction = true;
			}			
			UGameplayStatics::SpawnSoundAttached(FoundTargetSFX, RootComponent);
		}		
	}	
}

USHealthComponent* ASTrackerBall::I_GetHealthComp() const
{
	return HealthComp;
}
