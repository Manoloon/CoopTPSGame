// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBall.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Sound/SoundCue.h"
#include "components/AudioComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

// debug
static int32 DebugtrackballDraw = 0;
FAutoConsoleVariableRef CVARDebugtrackballDraw(
	TEXT("Coop.DebugTrackball"),
	DebugtrackballDraw,
	TEXT("draw debug lines for trackball"),
	ECVF_Cheat);


// Sets default values
ASTrackerBall::ASTrackerBall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
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
	MovementForce = 7000.0f;
	RequireDistanceToTarget = 100.0f;
	ExplosionDamage = 80.0f;
	ExplosionRadius = 600.0f;
	bExploded = false;
	bStartedSelfDestruction = false;
	SelfDamageInterval = 0.25;
}

// Called when the game starts or when spawned
void ASTrackerBall::BeginPlay()
{
	Super::BeginPlay();
	
	if(Role == ROLE_Authority)
	{
		// Initial location
		NextPathPoint = GetNextPathPoint();
	}
	// health
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBall::OnHealthChanged);
}

FVector ASTrackerBall::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX; // asignamos el maximo en float con ese macro.

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn  == nullptr && USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}
		USHealthComponent*  TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if(HealthComp && HealthComp->GetHealth()>0.0)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if(Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if(BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		// Con esto busca desatorarse si asi fuera recalculando el path.
		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBall::RefreshPath, 5.0f, false);

		if (NavPath->PathPoints.Num() > 1)
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
	if(MatInst == nullptr)
	{
	MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(1));
	}
	if(MatInst)
	{
	MatInst->SetScalarParameterValue("DamageTaken",GetWorld()->TimeSeconds);
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
	const UWorld* World = GetWorld();
	UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionFX, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, GetActorLocation());
	// escondemos el mesh al morir
	MeshComp->SetVisibility(false, true); 
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(Role == ROLE_Authority)
	{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this,GetInstigatorController(),true);

		if(DebugtrackballDraw)
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

// Called every frame
void ASTrackerBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(Role == ROLE_Authority && !bExploded)
	{
	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();
	if(GetVelocity().Size()>10)
	{
		AudioComp->SetSound(RollSFX);
	}
	if(DistanceToTarget <= RequireDistanceToTarget)
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
		if (DebugtrackballDraw)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 10, 12, FColor::Yellow, false, 1.0f, 0, 3.0f);
		}
	}
}

void ASTrackerBall::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if(!bStartedSelfDestruction)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor,this))
		{
			// we overlapped player
			if (Role == ROLE_Authority)
			{
				// set a timer that inflict damage to ourself until we self destruct --- ESTE TIMER se corre cada medio segundo , y aplica el daño de 10 a el mismo.
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBall::SelfDamage, SelfDamageInterval, true, 0.0f);
				bStartedSelfDestruction = true;
			}			
			UGameplayStatics::SpawnSoundAttached(FoundTargetSFX, RootComponent);
		}
		
	}
	
}