// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthNoRepComponent.h"
#include "Core/NetworkManager.h"
#include "Kismet/GameplayStatics.h"



// Sets default values for this component's properties
UHealthNoRepComponent::UHealthNoRepComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthNoRepComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only on server --- Como es un componente , no tiene role, sino que su dueño lo tiene.
	//** NetWork
	NetworkManager = Cast<ANetworkManager>(UGameplayStatics::GetActorOfClass(
										this,ANetworkManager::StaticClass()));
	if(IsValid(NetworkManager))
	{
		NetworkManager->RegisterActor(GetOwner(),Encode());
	}
}

// Called every frame
void UHealthNoRepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UHealthNoRepComponent::DoDamage(float Damage)
{
	if(GetOwner()->HasAuthority())
	{
		Health -=Damage;
		if (Health <= 0)
		{
			Health = 0;
			bOwnerIsDead=true;
		}
	
		if (IsValid(NetworkManager))
		{
			NetworkManager->UpdateActor(GetOwner(),Encode());
		}
	}
}

//** NETWORK MANAGER
void UHealthNoRepComponent::PostReplication(TArray<uint8> Payload)
{
	Decode(Payload);
	if (Health <=0)
	{
		UE_LOG(LogTemp,Warning,TEXT("DIEEEE"));
		bOwnerIsDead = true;
	}
}
// Server Function
TArray<uint8> UHealthNoRepComponent::Encode()
{
	TArray<uint8> Payload;
	FMemoryWriter Ar(Payload);
	Ar << Health;
	return Payload;
}
// Client Function
void UHealthNoRepComponent::Decode(const TArray<uint8>& Payload)
{
	FMemoryReader Ar(Payload);
	Ar << Health;
}