// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickup.h"
#include "SPowerUp.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"

// Sets default values
ASPickup::ASPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(75.0f);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(SphereComp);
	DecalComp->SetRelativeRotation(FRotator(0.0f,90.0f,0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);

	CooldDownDuraction = 10.0f;

}

// Called when the game starts or when spawned
void ASPickup::BeginPlay()
{
	Super::BeginPlay();
	RespawnPowerUp();
}

void ASPickup::RespawnPowerUp()
{
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO POWERUP Clas Set"));
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PowerupInstance = GetWorld()->SpawnActor<ASPowerUp>(PowerupClass, GetTransform(), SpawnParams);
}

void ASPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if(PowerupInstance)
	{
		PowerupInstance->ActivatePowerUp();
		PowerupInstance = nullptr;

		// set timer for respawn powerup.

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickup::RespawnPowerUp, CooldDownDuraction);
	}
}
