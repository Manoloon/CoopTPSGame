// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/TPSHud.h"
#include "Engine/Canvas.h"
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
/*
	if(const ASCharacter* Pawn = Cast<ASCharacter>(GetOwningPawn()); Pawn && Canvas)
	{
		DrawPlayerColorBar(Pawn->PlayerColor, false, 4.0f);
		DrawPlayerColorBar(Pawn->PlayerColor, true, 8.0f);
		if(Pawn->CurrentWeapon)
		{
			const float TargetExpandedWeight = Pawn->CurrentWeapon->bIsReloading ? 1.0f : 0.0f;
			CrossHairExpandWeight = FMath::FInterpTo(CrossHairExpandWeight, TargetExpandedWeight,
																	RenderDelta, 8.0f);
		}

		const float CrosshairSize = FMath::Lerp(16.0f, 22.0f, CrossHairExpandWeight);
		const float CrosshairGapSize = FMath::Lerp(6.0f, 20.0f, CrossHairExpandWeight);
		DrawCrosshair(Pawn->PlayerColor * 1.33f, CrosshairSize, CrosshairGapSize);
	}
	*/
}

void ATPSHud::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, 
FLinearColor Color)
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
/*
void ATPSHud::DrawPlayerColorBar(const FLinearColor& Color,const bool bBottom,const float Height)
{
	check(Canvas);

	constexpr float OriginX = 0.0f;
	const float OriginY = bBottom ? Canvas->SizeY - Height : 0.0f;
	DrawRect(Color, OriginX, OriginY, Canvas->SizeX, Height);
}

void ATPSHud::DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize)
{
	check(Canvas);

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	const float ArmOffset = TotalSize * 0.5f;
	const float GapOffset = GapSize * 0.5f;
	const float Thickness = 2.0f;

	DrawLine(CenterX - ArmOffset, CenterY, CenterX - GapOffset,
															CenterY, Color, Thickness);
	DrawLine(CenterX + GapOffset, CenterY, CenterX + ArmOffset,
															CenterY, Color, Thickness);
	DrawLine(CenterX, CenterY - ArmOffset, CenterX, CenterY
																	- GapOffset, Color, Thickness);
	DrawLine(CenterX, CenterY + GapOffset, CenterX, CenterY
																	+ ArmOffset, Color, Thickness);
}

void ATPSHud::DrawHealthBar(const FLinearColor& Color, float Height, float MaxHealth, float CurrentHealth)
{
	check(Canvas);
}*/
