// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/MainMenuInterface.h"
#include "Blueprint/UserWidget.h"
#include "BaseMenuWidget.generated.h"

UCLASS()
class COOPTPS_API UBaseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup();
	void Teardown();
	void SetMenuInterface(IMainMenuInterface* NewMenuInterface);

protected:
	IMainMenuInterface* MenuInterface;	
};
