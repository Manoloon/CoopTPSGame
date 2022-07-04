// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UPlayerUI.generated.h"

class USHealthComponent;
UCLASS()
class UUPlayerUI : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void PostInitProperties() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;
	UPROPERTY(meta = (BindWidget))
		UImage* Img_HealthIndicator;
	UPROPERTY()
	APawn* PlayerPawn;
public:
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ScoreVal;
	void SetHealthIndicator(const float NewHealth=1.0f) const;
	UFUNCTION()
	void HealthChanged(USHealthComponent* OwningHealthComp, const float Health,float HealthDelta,
					const class UDamageType* DamageType, class AController* InstigatedBy,AActor* DamageCauser);
};
