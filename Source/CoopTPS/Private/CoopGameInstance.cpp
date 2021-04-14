// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopGameInstance.h"
#include "Uobject/ConstructorHelpers.h"
#include "UI/UW_MainMenu.h"
#include "UI/W_PauseMenu.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/BaseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

UCoopGameInstance::UCoopGameInstance(const FObjectInitializer & ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UBaseMenuWidget>BPMainMenuClass(TEXT("/Game/UI/WG_MainMenu"));
	if(BPMainMenuClass.Class)
	{
		MainMenuClass = BPMainMenuClass.Class;
	}	
	static ConstructorHelpers::FClassFinder<UBaseMenuWidget>BPPauseMenuClass(TEXT("/Game/UI/WG_PauseMenu"));
	if (BPPauseMenuClass.Class)
	{
		PauseMenuClass = BPPauseMenuClass.Class;
	}
}

void UCoopGameInstance::Init()
{

}

void UCoopGameInstance::Host()
{
	if(MainMenu != nullptr )
	{
		MainMenu->Teardown();
	}
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Hosting")));
	UWorld* World = GetWorld();
	if(World)
	{
		World->ServerTravel("/Game/Map/M_Level1?listen");
	}
}

void UCoopGameInstance::Join(const FString& Address)
{
	if (MainMenu != nullptr && Address.IsValidIndex(7))
	{
		MainMenu->Teardown();
	}
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void UCoopGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel("/Game/Map/M_MainMenu", ETravelType::TRAVEL_Absolute);
	}
}

void UCoopGameInstance::QuitGame()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if(PlayerController)
	{
		UEngine* Engine = GetEngine();
		if (!ensure(Engine != nullptr)) return;
		Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Quitting Game")));
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, true);
	}
}

void UCoopGameInstance::LoadMenu()
{
	if(MainMenuClass)
	{
		MainMenu = CreateWidget<UBaseMenuWidget>(this, MainMenuClass);
		if (MainMenu == nullptr) { return; }
		MainMenu->Setup();
		MainMenu->SetMenuInterface(this);
	}	
}

void UCoopGameInstance::LoadPauseMenu()
{
	if (PauseMenuClass)
	{
		PauseMenu = CreateWidget<UBaseMenuWidget>(this, PauseMenuClass);
		if (PauseMenu == nullptr) { return; }
		PauseMenu->Setup();
		PauseMenu->SetMenuInterface(this);
	}
}
