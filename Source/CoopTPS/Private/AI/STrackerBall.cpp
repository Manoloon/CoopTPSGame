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

// debug

static int32 DebugTrackballDraw = 0;
/*
FAutoConsoleVariableRef CVARDebugtrackballDraw(
	TEXT("Coop.DebugTrackball"),
	DebugTrackballDraw,
	TEXT("draw debug lines for trackball"),
	ECVF_Cheat);
*/

// Sets default values
ASTrackerBall::ASTrackerBall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	//MeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealhComp"));
	
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

// Called when the game starts or when spawned
void ASTrackerBall::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		NextPathPoint = GetNextPathPoint();
	}
	// health
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBall::OnHealthChanged);
}


void ASTrackerBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
			// keep going toward goal
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bVelocityChanged);
		}

		if (DebugTrackballDraw)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 10, 12, FColor::Yellow, false, 1.0f, 0, 3.0f);
		}
	}
}

FVector ASTrackerBall::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX; // asignamos el maximo en float con ese macro.

	for (TActorIterator<APawn>PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		APawn* TestPawn = *PawnIterator;
		if (TestPawn  == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}
		const USHealthComponent*  TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if(TestPawnHealthComp && TestPawnHealthComp->GetHealth()>0.0)
		{
			const float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if(Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if(BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);
		if (!NavPath) { return GetActorLocation(); }
		// Con esto busca desatorarse si asi fuera recalculando el path.
		FTimerHandle RefreshPathTH;
		GetWorldTimerManager().ClearTimer(RefreshPathTH);
		GetWorldTimerManager().SetTimer(RefreshPathTH, this, &ASTrackerBall::RefreshPath, 5.0f, false);
		
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			return NavPath->PathPoints[1];
		}
	}	
		return GetActorLocation();	
}

void ASTrackerBall::SelfDamage()
{
	UGameplayStatics::ApplyDamage(this, 10.0f, GetInstigatorController(), this, nullptr);
}

void ASTrackerBall::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	/*
 	if(MeshMaterialInstance == nullptr)
 	{
 	MeshMaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
 	}
 	if(MeshMaterialInstance)
 	{
 	MeshMaterialInstance->SetScalarParameterValue("DamageTaken",GetWorld()->TimeSeconds);
 	}
	// si hittimer not playing
	//GetWorldTimerManager().SetTimer(HitShakeTH,this, &ThisClass::StartHitShake, 0.0f, false);
	*/
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
	// escondemos el mesh al morir -> y el true es para la propagacion. 
	MeshComp->SetVisibility(false, true); 
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(HasAuthority())
	{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this,GetInstigatorController(),true);

		if(DebugTrackballDraw)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 3.0f);
		}
	 // en vez de Destroy() , usamos lifespan para que le de tiempo al cliente de ver la explosion.
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
		MeshMaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MeshMaterialInstance)
	{
		const float DamageHit = FMath::FInterpTo(1.0f, 0.0, GetWorld()->TimeSeconds, 1.0f);
		MeshMaterialInstance->SetScalarParameterValue("DamageTaken", DamageHit);
		if(DamageHit == 0.0f)
		{
			GetWorldTimerManager().ClearTimer(HitShakeTH);
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
				FTimerHandle SelfDamageTH;
				// set a timer that inflict damage to ourself until we self destruct --- ESTE TIMER se corre cada medio segundo , y aplica el daño de 10 a el mismo.
				GetWorldTimerManager().SetTimer(SelfDamageTH, this, &ASTrackerBall::SelfDamage, SelfDamageInterval, true, 0.0f);
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
