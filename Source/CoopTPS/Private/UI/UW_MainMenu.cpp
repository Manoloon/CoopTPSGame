// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_MainMenu.h"
#include "Components/Button.h"

void UUW_MainMenu::SetMenuInterface(IMainMenuInterface* NewMenuInterface)
{
	MainMenuInterface = NewMenuInterface;
}

void UUW_MainMenu::Setup()
{
	this->AddToViewport();
	this->bIsFocusable = true;
	UWorld* World = GetWorld();
	if (World == nullptr) { return; }
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}
}

void UUW_MainMenu::TearDown()
{	
	this->RemoveFromParent();
	UWorld* World = GetWorld();
	if (World == nullptr) { return; }
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(true);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}

bool UUW_MainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { return false; }
	BTN_Host->OnClicked.AddDynamic(this, &UUW_MainMenu::HostServer);
	return true;
}

void UUW_MainMenu::HostServer()
{
	if(MainMenuInterface !=nullptr)
	{
		MainMenuInterface->Host();
	}
}