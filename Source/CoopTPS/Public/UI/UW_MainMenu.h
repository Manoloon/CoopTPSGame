// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMenuWidget.h"
#include "UW_MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UUW_MainMenu : public UBaseMenuWidget
{
	GENERATED_BODY()
public:


protected:
	virtual bool Initialize() override;
private:
	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_Join;
	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_Host;
 	UPROPERTY(meta = (BindWidget))
 		class UButton* BTN_Connect;
 	UPROPERTY(meta = (BindWidget))
 		class UButton* BTN_Cancel;

	UPROPERTY(meta = (BindWidget))
		class UWidgetSwitcher* JoinWidgetSwitcher;
	UPROPERTY(meta = (BindWidget))
		class UEditableText* IpAddressText;

	// estos son los widgets que estan dentro del switcher.
 	UPROPERTY(meta = (BindWidget))
 		class UWidget* JoinMenu;
	UPROPERTY(meta = (BindWidget))
		class UWidget* MainMenu;

	UFUNCTION()
		void HostServer();
	UFUNCTION()
		void OpenJoinMenu();
	UFUNCTION()
		void BackToMainMenu();
	UFUNCTION()
		void JoinServer();
};
