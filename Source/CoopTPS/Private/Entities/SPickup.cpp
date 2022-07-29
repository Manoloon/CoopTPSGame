// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/SPickup.h"
#include "Entities/SPowerUp.h"
#include "Entities/SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"

ASPickup::ASPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(75.0f);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(SphereComp);
	DecalComp->SetRelativeRotation(FRotator(0.0f,90.0f,0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);

	CoolDownDuration = 10.0f;

}

void ASPickup::BeginPlay()
{
	Super::BeginPlay();
	RespawnPowerUp();
}

void ASPickup::RespawnPowerUp()
{
	if (PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO POWERUP Clas Set"));
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PowerUpInstance = GetWorld()->SpawnActor<ASPowerUp>(PowerUpClass, GetTransform(), SpawnParams);
}

void ASPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	/*ASCharacter* OtherCharacter = Cast<ASCharacter>(OtherActor);*/
	if(OtherActor == UGameplayStatics::GetPlayerPawn(this,0))
	{
		if (PowerUpInstance)
		{
			PowerUpInstance->ActivatePowerUp();
			PowerUpInstance = nullptr;

			GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickup::RespawnPowerUp, CoolDownDuration);
		}
	}	
}
