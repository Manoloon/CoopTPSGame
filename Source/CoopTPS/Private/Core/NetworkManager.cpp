// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/NetworkManager.h"
#include "Engine/NetSerialization.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANetworkManager::ANetworkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates=true;
	bAlwaysRelevant = true;

}

void ANetworkManager::RegisterActor(AActor* MyActor, const TArray<uint8>& Payload)
{
	if(const UHealthNoRepComponent* Hc = MyActor->FindComponentByClass<UHealthNoRepComponent>())
	{
		const FHealthCompItem healthCompItem(Hc,Payload);
		HealthTracker.MarkItemDirty(HealthTracker.Items.Add_GetRef(healthCompItem));
	}
}

void ANetworkManager::UpdateActor(AActor* MyActor, const TArray<uint8>& Payload)
{
	FHealthCompItem* FoundEntry = HealthTracker.Items.FindByPredicate([MyActor](const FHealthCompItem& InItem)
	{
		return InItem.OwnerActor == MyActor;
	});
	FoundEntry->Data = Payload;
	HealthTracker.MarkItemDirty(*FoundEntry);
}

void ANetworkManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetworkManager,HealthTracker);
}

// Called when the game starts or when spawned
void ANetworkManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetworkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

