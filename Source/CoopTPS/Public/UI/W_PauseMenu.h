// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMenuWidget.h"
#include "W_PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UW_PauseMenu : public UBaseMenuWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_Cancel;
	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_Quit;

	UFUNCTION()
		void BackToGame();
	UFUNCTION()
		void BackToMainMenu();

private:
	virtual bool Initialize() override;
};
