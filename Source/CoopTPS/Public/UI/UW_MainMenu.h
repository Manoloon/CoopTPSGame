// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenuInterface.h"
#include "Blueprint/UserWidget.h"
#include "UW_MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UUW_MainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMenuInterface(IMainMenuInterface* NewMenuInterface);
	void Setup();
	void TearDown();
protected:
	virtual bool Initialize() override;
private:
	UPROPERTY(META = (BindWidget))
		class UButton* BTN_Join;
	UPROPERTY(META = (BindWidget))
		class UButton* BTN_Host;
	UFUNCTION()
		void HostServer();

protected:

	IMainMenuInterface* MainMenuInterface;
};
