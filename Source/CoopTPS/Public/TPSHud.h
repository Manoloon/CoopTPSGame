// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TPSHud.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API ATPSHud : public AHUD
{
	GENERATED_BODY()
	
public:
	float CrossHairExpandWeight;

public:
	virtual void DrawHUD() override;

private:
	void DrawPlayerColorBar(const FLinearColor& Color, bool bBottom, float Height);
	void DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize);
	void DrawHealthBar(const FLinearColor& Color, float Height, float MaxHealth,float CurrentHealth);

};
