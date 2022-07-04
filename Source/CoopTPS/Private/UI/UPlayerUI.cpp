// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UPlayerUI.h"
#include "Interfaces/IHealthyActor.h"
#include "Components/SHealthComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UUPlayerUI::PostInitProperties()
{
	Super::PostInitProperties();
	SetHealthIndicator(1.f);
}

void UUPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerPawn = GetOwningPlayerPawn();
	if(PlayerPawn)
	{
		if(PlayerPawn->Implements<UIHealthyActor>())
		{
			const auto I = Cast<IIHealthyActor>(PlayerPawn);
			I->I_GetHealthComp()->OnHealthChanged.AddDynamic(this, &UUPlayerUI::HealthChanged);
		}
	}
}

void UUPlayerUI::BeginDestroy()
{
	if(PlayerPawn)
	{
		if(PlayerPawn->Implements<UIHealthyActor>())
		{
			const auto I = Cast<IIHealthyActor>(PlayerPawn);
			UE_LOG(LogTemp,Error,TEXT("Removing Dynamic"));
			I->I_GetHealthComp()->OnHealthChanged.RemoveDynamic(this,&UUPlayerUI::HealthChanged);
		}
	}
	Super::BeginDestroy();
}

void UUPlayerUI::SetHealthIndicator(const float NewHealth) const
{
	if(IsValid(Img_HealthIndicator))
	{
		const auto HealthDynMat = Img_HealthIndicator->GetDynamicMaterial();
		HealthDynMat->SetScalarParameterValue(FName("Alpha"),NewHealth);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UUPlayerUI::HealthChanged(USHealthComponent* OwningHealthComp, const float Health, float HealthDelta,
                               const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	
	if(PlayerPawn)
	{
		if(PlayerPawn->Implements<UIHealthyActor>())
		{
			const auto I = Cast<IIHealthyActor>(PlayerPawn);
			const float MHealth =  I->I_GetHealthComp()->MaxHealth;
			const float ValueCorrected = 1.f - (UKismetMathLibrary::SafeDivide(Health,MHealth));
			const float Value =FMath::Clamp(ValueCorrected,0.0,1.0);
			SetHealthIndicator(Value);
		}
	}
}
	
