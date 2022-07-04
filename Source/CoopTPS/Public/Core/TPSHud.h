// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/ST_HUDData.h"
#include "TPSHud.generated.h"

UCLASS()
class COOPTPS_API ATPSHud final : public AHUD
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,Category = "Settings")
	TSubclassOf<UUserWidget> PlayerUIClass;
	UPROPERTY(EditAnywhere,Category = "Settings")
	TSubclassOf<UUserWidget> AnnouncementClass;
	UPROPERTY(EditAnywhere,Category = "Settings")
	TSubclassOf<UUserWidget> DebugRoleMessageClass;
	UPROPERTY()
	class UUPlayerUI* PlayerUI;
	UPROPERTY()
	class URoleMessage* RoleMessage;
	UPROPERTY()
	UUserWidget* Announcements;
	// announcement pointer.
	virtual void BeginPlay() override;
	FHUDData HUDData;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax=16.0f;
	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread,
													FLinearColor Color=FLinearColor::White);
public:
	float CrossHairExpandWeight;
	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDData(const FHUDData& Data){HUDData = Data;}
	UUPlayerUI* GetPlayerUI() const;
	URoleMessage* GetRoleMessage() const;
	void AddRoleMessage();
	void AddPlayerUI();
	void AddAnnouncement();
	//void HideAllWidgets();
};
