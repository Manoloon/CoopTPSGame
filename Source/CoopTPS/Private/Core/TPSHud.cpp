// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/TPSHud.h"
#include "Engine/Canvas.h"
#include "UserWidget.h"
#include "UI/UPlayerUI.h"
#include "Entities/SCharacter.h"

void ATPSHud::DrawHUD()
{
	Super::DrawHUD();
	if(GEngine)
	{
		FVector2d ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X/2.0f,ViewportSize.Y/2.0f);
		const float SpreadScaled = CrosshairSpreadMax * HUDData.CrosshairSpread;
		if(HUDData.CrosshairsCenter)
		{
			const FVector2d Spread(0.f,0.f);
			DrawCrosshair(HUDData.CrosshairsCenter,ViewportCenter,Spread,HUDData.CrosshairColor);
		}
		if(HUDData.CrosshairsLeft)
		{
			const FVector2d Spread(-SpreadScaled,0.f);
			DrawCrosshair(HUDData.CrosshairsLeft,ViewportCenter,Spread,HUDData.CrosshairColor);
		}
		if(HUDData.CrosshairsRight)
		{
			const FVector2d Spread(SpreadScaled,0.f);
			DrawCrosshair(HUDData.CrosshairsRight,ViewportCenter,Spread,HUDData.CrosshairColor);
		}
		if(HUDData.CrosshairsUp)
		{
			const FVector2d Spread(0.f,-SpreadScaled);
			DrawCrosshair(HUDData.CrosshairsUp,ViewportCenter,Spread,HUDData.CrosshairColor);
		}
		if(HUDData.CrosshairsDown)
		{
			const FVector2d Spread(0.f,SpreadScaled);
			DrawCrosshair(HUDData.CrosshairsDown,ViewportCenter,Spread,HUDData.CrosshairColor);
		}
	}
}

UUPlayerUI* ATPSHud::GetPlayerUI() const
{
	return PlayerUI;
}

UUserWidget* ATPSHud::GetHealthIndicator() const
{
	return HealthIndicator;
}

void ATPSHud::AddPlayerUI()
{
	if(GetOwningPlayerController() && PlayerUIClass)
	{
		PlayerUI = CreateWidget<UUPlayerUI>(GetOwningPlayerController(),PlayerUIClass);
		
		PlayerUI->AddToViewport();
	}
}

void ATPSHud::AddHealthIndicator()
{
	if(GetOwningPlayerController() && HealthIndicatorClass)
	{
		HealthIndicator = CreateWidget<UUserWidget>(GetWorld(),HealthIndicatorClass);
		HealthIndicator->AddToViewport();
	}
}

void ATPSHud::AddAnnouncement()
{
	if(GetOwningPlayerController() && AnnouncementClass)
	{
		// create widget de announcement
		// add to viewport
	}
}

void ATPSHud::DrawCrosshair(UTexture2D* Texture, const FVector2d ViewportCenter, const FVector2d Spread,
                            const FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d TextureDrawPoint(
		ViewportCenter.X - (TextureWidth /2.0f) + Spread.X,
		ViewportCenter.Y - (TextureHeight /2.0f) + Spread.Y );
	DrawTexture(Texture,TextureDrawPoint.X,TextureDrawPoint.Y,
		TextureWidth,TextureHeight,0.f,0.f,
						1.f,1.f,Color);
}
