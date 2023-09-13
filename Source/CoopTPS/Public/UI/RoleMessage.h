// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoleMessage.generated.h"

class UTextBlock;
UCLASS(Abstract)
class URoleMessage final : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta =(BindWidget))
	UTextBlock* DisplayText;
	void SetDisplayText(FString TextToDisplay) const;
	void ShowPlayerNetRole(const APawn* Pawn) const;
protected:
	virtual void NativeDestruct() override;
};
