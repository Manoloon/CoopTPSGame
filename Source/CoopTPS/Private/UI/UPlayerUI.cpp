// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UPlayerUI.h"

#include "Components/TextBlock.h"
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

void UUPlayerUI::SetMatchTime(const FString NewTime) const
{
	GameTimeText->SetText(FText::FromString(NewTime));
}

void UUPlayerUI::SetPingIndicator(const float PingValue) const
{
	const FLinearColor CurrentColorPing = PingValue > 50.f ?
														FLinearColor{255,0,0,255} :
														FLinearColor{0,255,0,155};
	PingImg->SetBrushTintColor(CurrentColorPing);
}

void UUPlayerUI::SetWeaponName(const FName NewWeaponName) const
{
	WeaponNameText->SetText(FText::FromName(NewWeaponName));
}

void UUPlayerUI::SetCurrentAmmo(const int32 NewAmmo, const int32 NewMaxAmmo) const
{
	const FString Ammo = FString::Printf(TEXT("%02d:%02d"),NewAmmo,NewMaxAmmo);
	AmmoText->SetText(FText::FromString(Ammo));
	const FLinearColor TextColor = NewAmmo == 0 ? FColor::Red : FColor::Green;
	AmmoText->SetColorAndOpacity(TextColor);
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
	
