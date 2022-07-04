// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BaseMenuWidget.h"

void UBaseMenuWidget::Setup()
{
	this->AddToViewport();
	this->bIsFocusable = true;
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}
}

void UBaseMenuWidget::Teardown()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(true);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
	this->RemoveFromParent();
}

void UBaseMenuWidget::SetMenuInterface(IMainMenuInterface* NewMenuInterface)
{
	MenuInterface = NewMenuInterface;
}
