// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ThrowComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values for this component's properties
UThrowComponent::UThrowComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Throwing granade Arc
	BeamComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComp"));
	BeamComponent->bAutoActivate = false;
}


// Called when the game starts
void UThrowComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UThrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UThrowComponent::StarThrow()
{
}

void UThrowComponent::StopThrow()
{
	// cuando soltamos el boton de tirar granada se inicia esta funcion.
	// el launchdistance = a la distancia ultima.
	// seteamos la visibilidad del beamEndPointDecal
	// clear el beam array
	// throw function.
}

void UThrowComponent::Throw()
{
	// si la clase granada no es null
	// Clear el beam array
	// instancia un deferred de la granada usando el spawnrotation y el startlocation.
	// le seteamos la InitialLocalVelocity = a la initialLocalVelocity var.
	// UGameplayStatics::FinishSpawningActor(Grenade, FTransform(SpawnRotation, StartLocation, SpawnScale));
}

void UThrowComponent::ClearBeam()
{
	for(auto Beam = BeamArray.CreateIterator(); Beam;++Beam)
	{
		(*Beam)->DestroyComponent(true);
	}
	BeamArray.Empty();
}

void UThrowComponent::AddNewBeam(FVector newPoint1, FVector newPoint2)
{
	BeamComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BeamFX,newPoint1,FRotator::ZeroRotator,true);
	BeamArray.Add(BeamComponent);
	BeamComponent->SetBeamSourcePoint(0,newPoint1,0);
	BeamComponent->SetBeamTargetPoint(0,newPoint2,0);
}

void UThrowComponent::GetSegmentAtTime(const FVector LocStartLocation, const FVector LocInitialVelocity, const FVector LocGravity,
                                       const float Time1, const float Time2, FVector& OutPoint1, FVector& OutPoint2)
{
	OutPoint1 = (LocStartLocation + (LocInitialVelocity*Time1) + (LocGravity*(Time1*Time1*0.5f)));
	OutPoint2 = (LocStartLocation + (LocInitialVelocity*Time2) + (LocGravity*(Time2*Time2*0.5f)));
}

void UThrowComponent::DrawingTrajectory()
{
	/**
	ClearBeam();
	constexpr float PathLifeTime = 5.0f;
	SpawnRotation = GetControlRotation();
	FVector GrenadeOffset = FVector(100.0f, 0.0f, -10.0f);
	FVector ThrowRotateVector = GetControlRotation().RotateVector(GrenadeOffset);
	StartLocation = GetMesh()->GetSocketLocation(GrenadeSocketName) + ThrowRotateVector;
	const FTransform TotalPosition(SpawnRotation, ThrowRotateVector, SpawnScale);

	FVector InitialVelocity = UKismetMathLibrary::TransformDirection(TotalPosition, InitialLocalVelocity);
	uint8 LastIndex = floor(PathLifeTime / TimeInterval); 
	for (uint8 i = 0; i <= LastIndex; i++)
	{
		float Time1 = i * TimeInterval;
		float Time2 = (i + 1) * TimeInterval;

		const FName TraceTag("TraceTag");
		FCollisionQueryParams QueryParams;
		QueryParams.TraceTag = TraceTag;
		FHitResult Hit;
		GetSegmentAtTime(StartLocation, InitialVelocity, Gravity, Time1, Time2,Point1,Point2);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Point1, Point2, ECC_Visibility, QueryParams))
		{
			BeamEndPointDecal->SetVisibility(true);
			BeamEndPointDecal->SetWorldLocation(Hit.ImpactPoint);
			BeamEndPointDecal->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal));
								
			break;
		}
		AddNewBeam(Point1, Point2);
		BeamEndPointDecal->SetVisibility(false);
	}*/
}

