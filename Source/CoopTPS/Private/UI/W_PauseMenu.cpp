// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/W_PauseMenu.h"
#include "Components/button.h"


bool UW_PauseMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { return false; }
	if (!ensure(BTN_Cancel != nullptr)) return false;
	BTN_Cancel->OnClicked.AddDynamic(this, &UW_PauseMenu::BackToGame);
	if (!ensure(BTN_Quit != nullptr)) return false;
	BTN_Quit->OnClicked.AddDynamic(this, &UW_PauseMenu::BackToMainMenu);
	return true;
}

void UW_PauseMenu::BackToGame()
{
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
	this->RemoveFromParent();
}

void UW_PauseMenu::BackToMainMenu()
{
	if (MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("interface"));
		Teardown();
		MenuInterface->I_LoadMainMenu();
	}
	UE_LOG(LogTemp, Warning, TEXT("no menu interface"));
}

