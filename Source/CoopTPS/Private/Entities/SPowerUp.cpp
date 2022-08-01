// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/SPowerUp.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

ASPowerUp::ASPowerUp()
{
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComp;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->CastShadow = false;
	MeshComp->SetRelativeLocation(FVector(0.0, 0.0, 90.0));
	MeshComp->SetupAttachment(RootComponent);

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("LightComp"));
	LightComp->SetupAttachment(MeshComp);

	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
	
	PowerUpInterval = 0.0f;
	TotalNumTicks = 0.0f;
}

void ASPowerUp::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerUp::OnTickPowerUp()
{
	TicksProcess++;
	OnPowerUpTicked();
	
	if (TicksProcess >= TotalNumTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTicks);
	}
}

void ASPowerUp::ActivatePowerUp()
{
	OnActivated();

	if(PowerUpInterval >0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTicks, this,
									&ASPowerUp::OnTickPowerUp, PowerUpInterval, true);
	}
	else
	{
		OnTickPowerUp();
	}
	
}
	
