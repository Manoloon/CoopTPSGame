// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopGameInstance.h"
#include "Uobject/ConstructorHelpers.h"
#include "UI/UW_MainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

UCoopGameInstance::UCoopGameInstance(const FObjectInitializer & ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget>BPMainMenuClass(TEXT("/Game/UI/WG_MainMenu"));
	if(BPMainMenuClass.Class)
	{
		MainMenuClass = BPMainMenuClass.Class;
	}	
}

void UCoopGameInstance::Init()
{

}

void UCoopGameInstance::Host()
{
	if(MainMenu != nullptr )
	{
		MainMenu->TearDown();
	}
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Hosting")));
	UWorld* World = GetWorld();
	if(World)
	{
		World->ServerTravel("/Game/Map/M_Level1");
	}
}

void UCoopGameInstance::Join(const FString& Address)
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if(PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void UCoopGameInstance::LoadMenu()
{
	if(MainMenuClass)
	{
		MainMenu = CreateWidget<UUW_MainMenu>(this, MainMenuClass);
		if (MainMenu == nullptr) { return; }
		MainMenu->Setup();
		MainMenu->SetMenuInterface(this);
	}	
}
