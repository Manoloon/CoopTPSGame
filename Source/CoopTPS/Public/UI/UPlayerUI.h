// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UPlayerUI.generated.h"

class USHealthComponent;
class UTextBlock;
UCLASS()
class UUPlayerUI : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void PostInitProperties() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;
	
	UPROPERTY(meta = (BindWidget))
	UImage* Img_HealthIndicator;
	UPROPERTY(meta = (BindWidget))
	UImage* PingImg;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameTimeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;
	UPROPERTY()
	APawn* PlayerPawn;
	// TODO : text how many kills
	
public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreVal;
	void SetHealthIndicator(const float NewHealth=1.0f) const;
	void SetMatchTime(const FString NewTime) const;
	void SetPingIndicator(const float PingValue) const;
	void SetWeaponName(const FName NewWeaponName) const;
	void SetCurrentAmmo(const int32 NewAmmo,const int32 NewMaxAmmo=20) const;
	UFUNCTION()
	void HealthChanged(USHealthComponent* OwningHealthComp, const float Health,float HealthDelta,
					const class UDamageType* DamageType, class AController* InstigatedBy,AActor* DamageCauser);
};
