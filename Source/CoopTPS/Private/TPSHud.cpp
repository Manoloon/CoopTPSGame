// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSHud.h"
#include "Engine/Canvas.h"
#include "SWeapon.h"
#include "SCharacter.h"
#include "SHealthComponent.h"

void ATPSHud::DrawHUD()
{
	Super::DrawHUD();

	ASCharacter* Pawn = Cast<ASCharacter>(GetOwningPawn());
	if(Pawn && Canvas)
	{
		DrawPlayerColorBar(Pawn->PlayerColor, false, 4.0f);
		DrawPlayerColorBar(Pawn->PlayerColor, true, 8.0f);
		if(Pawn->CurrentWeapon)
		{
			const float TargetExpandedWeight = Pawn->CurrentWeapon->bIsReloading ? 1.0f : 0.0f;
			CrossHairExpandWeight = FMath::FInterpTo(CrossHairExpandWeight, TargetExpandedWeight, RenderDelta, 8.0f);
		}

		const float CrosshairSize = FMath::Lerp(16.0f, 22.0f, CrossHairExpandWeight);
		const float CrosshairGapSize = FMath::Lerp(6.0f, 20.0f, CrossHairExpandWeight);
		DrawCrosshair(Pawn->PlayerColor * 1.33f, CrosshairSize, CrosshairGapSize);
	}
}

void ATPSHud::DrawPlayerColorBar(const FLinearColor& Color, bool bBottom, float Height)
{
	check(Canvas);

	const float OriginX = 0.0f;
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

	DrawLine(CenterX - ArmOffset, CenterY, CenterX - GapOffset, CenterY, Color, Thickness);
	DrawLine(CenterX + GapOffset, CenterY, CenterX + ArmOffset, CenterY, Color, Thickness);
	DrawLine(CenterX, CenterY - ArmOffset, CenterX, CenterY - GapOffset, Color, Thickness);
	DrawLine(CenterX, CenterY + GapOffset, CenterX, CenterY + ArmOffset, Color, Thickness);
}

void ATPSHud::DrawHealthBar(const FLinearColor& Color, float Height, float MaxHealth, float CurrentHealth)
{
	check(Canvas);
}
