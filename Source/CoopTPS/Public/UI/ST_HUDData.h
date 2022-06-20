#pragma once

#include "ST_HUDData.generated.h"

USTRUCT()
struct FHUDData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairsCenter;
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairsUp;
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairsDown;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
	FHUDData()
	{
		CrosshairsCenter=nullptr;
		CrosshairsLeft=nullptr;
		CrosshairsRight=nullptr;
		CrosshairsUp=nullptr;
		CrosshairsDown=nullptr;
		CrosshairSpread=10.0f;
		CrosshairColor = FLinearColor::White;
	};
};