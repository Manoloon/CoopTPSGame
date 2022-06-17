// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/ST_HUDData.h"
#include "TPSHud.generated.h"

UCLASS()
class COOPTPS_API ATPSHud : public AHUD
{
	GENERATED_BODY()
	
public:
	float CrossHairExpandWeight;
	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDData(const FHUDData& Data){HUDData = Data;}
private:
	FHUDData HUDData;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax=16.0f;
	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread);
	
	//void DrawPlayerColorBar(const FLinearColor& Color,const bool bBottom,const float Height);
	//void DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize);
	//void DrawHealthBar(const FLinearColor& Color, float Height, float MaxHealth,float CurrentHealth);

};
