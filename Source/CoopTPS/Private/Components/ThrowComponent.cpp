// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ThrowComponent.h"

// Sets default values for this component's properties
UThrowComponent::UThrowComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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

	// ...
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

}

void UThrowComponent::AddNewBeam(FVector newPoint1, FVector newPoint2)
{

}

