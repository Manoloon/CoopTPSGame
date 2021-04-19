// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_MainMenu.h"
#include "Components/Button.h"
#include "Uobject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"


UUW_MainMenu::UUW_MainMenu(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget>BPServerListItemClass(TEXT("/Game/UI/UWG_ServerItem"));
	if(BPServerListItemClass.Class)
	{
		ServerListItemClass = BPServerListItemClass.Class;
	}
}

// inicializa el widget y crea el binding con la funcion hostServer y al menu de join server.
bool UUW_MainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { return false; }
	if (!ensure(BTN_Host != nullptr)) return false;
	BTN_Host->OnClicked.AddDynamic(this, &UUW_MainMenu::HostServer);
	if (!ensure(BTN_Join != nullptr)) return false;
	BTN_Join->OnClicked.AddDynamic(this, &UUW_MainMenu::OpenJoinMenu);
	if (!ensure(BTN_Cancel != nullptr)) return false;
	BTN_Cancel->OnClicked.AddDynamic(this, &UUW_MainMenu::BackToMainMenu);
	if(!ensure(BTN_Connect !=nullptr))return false;
	BTN_Connect->OnClicked.AddDynamic(this, &UUW_MainMenu::JoinServer);
	if (!ensure(BTN_Quit != nullptr))return false;
	BTN_Quit->OnClicked.AddDynamic(this, &UUW_MainMenu::QuitGame);

	return true;
}

// llama a la funcion de la interface.
void UUW_MainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
		MenuInterface->Host();
	}

}
// abre el menu de join -> ip address call.
void UUW_MainMenu::OpenJoinMenu()
{
 	if (!ensure(JoinWidgetSwitcher != nullptr)) return;
 	if (!ensure(JoinMenu != nullptr))return;
	JoinWidgetSwitcher->SetActiveWidget(JoinMenu);
	if(MenuInterface !=nullptr)
	{
		MenuInterface->RefreshServerList();
	}
}

void UUW_MainMenu::BackToMainMenu()
{
	if (!ensure(JoinWidgetSwitcher != nullptr)) return;
	if (!ensure(MainMenu != nullptr))return;
	JoinWidgetSwitcher->SetActiveWidget(MainMenu);
}

void UUW_MainMenu::JoinServer()
{
	if(SelectedIndex.IsSet())
	{
		if(MenuInterface != nullptr)
		{
			MenuInterface->Join(SelectedIndex.GetValue());
		}
	}
}

void UUW_MainMenu::QuitGame()
{
	if (MenuInterface != nullptr)
	{
		MenuInterface->QuitGame();
	}
}

void UUW_MainMenu::SetServerListItems(TArray<FString>newServerNames)
{
	ServerList->ClearChildren();
	uint32 LocalIndex = 0;
	for(const FString& ServerName : newServerNames)
	{
		++LocalIndex;
	}
}

void UUW_MainMenu::SetSelectedIndex(uint32 newIndex)
{
	SelectedIndex = newIndex;
}