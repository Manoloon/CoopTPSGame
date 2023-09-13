// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_MainMenu.h"
#include "Components/Button.h"
#include "UI/W_ServerListItem.h"
#include "Uobject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"


UUW_MainMenu::UUW_MainMenu(const FObjectInitializer& ObjectInitializer)
	: BTN_Join(nullptr), BTN_Host(nullptr), BTN_Connect(nullptr), BTN_Cancel(nullptr), BTN_Quit(nullptr),
		BTN_CreateSession(nullptr), BTN_HostCancel(nullptr), JoinWidgetSwitcher(nullptr), ServerList(nullptr),
		NameServerBox(nullptr), JoinMenu(nullptr), MainMenu(nullptr), HostMenu(nullptr)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BPServerListItemClass(TEXT("/Game/UI/UWG_ServerItem"));
	if (BPServerListItemClass.Class)
	{
		ServerListItemClass = BPServerListItemClass.Class;
	}
}

bool UUW_MainMenu::Initialize()
{
	const bool Success = Super::Initialize();
	if (!Success) { return false; }
	if (!ensure(BTN_Host != nullptr)) return false;
	BTN_Host->OnClicked.AddDynamic(this, &UUW_MainMenu::OpenHostMenu);
	if (!ensure(BTN_Join != nullptr)) return false;
	BTN_Join->OnClicked.AddDynamic(this, &UUW_MainMenu::OpenJoinMenu);
	if (!ensure(BTN_Cancel != nullptr)) return false;
	BTN_Cancel->OnClicked.AddDynamic(this, &UUW_MainMenu::BackToMainMenu);
	if(!ensure(BTN_Connect !=nullptr))return false;
	BTN_Connect->OnClicked.AddDynamic(this, &UUW_MainMenu::JoinServer);
	if (!ensure(BTN_Quit != nullptr))return false;
	BTN_Quit->OnClicked.AddDynamic(this, &UUW_MainMenu::QuitGame);
	if (!ensure(BTN_CreateSession != nullptr)) return false;
	BTN_CreateSession->OnClicked.AddDynamic(this, &UUW_MainMenu::HostServer);
	if (!ensure(BTN_HostCancel != nullptr)) return false;
	BTN_HostCancel->OnClicked.AddDynamic(this, &UUW_MainMenu::BackToMainMenu);

	return true;
}

// llama a la funcion de la interface.
// ReSharper disable once CppMemberFunctionMayBeConst
void UUW_MainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
		const FString newServerName = NameServerBox->GetText().ToString();
		MenuInterface->I_Host(newServerName);
	}

}
// abre el menu de join -> ip address call.
// ReSharper disable once CppMemberFunctionMayBeConst
void UUW_MainMenu::OpenJoinMenu()
{
 	if (!ensure(JoinWidgetSwitcher != nullptr)) return;
 	if (!ensure(JoinMenu != nullptr))return;
	JoinWidgetSwitcher->SetActiveWidget(JoinMenu);
	if(MenuInterface !=nullptr)
	{
		MenuInterface->I_RefreshServerList();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
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
			MenuInterface->I_Join(SelectedIndex.GetValue());
		}
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void UUW_MainMenu::QuitGame()
{
	if (MenuInterface != nullptr)
	{
		MenuInterface->I_QuitGame();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void UUW_MainMenu::OpenHostMenu()
{
	if (!ensure(JoinWidgetSwitcher != nullptr)) return;
	if (!ensure(HostMenu != nullptr))return;
	JoinWidgetSwitcher->SetActiveWidget(HostMenu);
}

void UUW_MainMenu::UpdateServerListChildren()
{
	for(int32 i=0;i<ServerList->GetChildrenCount();i++)
	{
		const auto CurrentItem = Cast<UW_ServerListItem>(ServerList->GetChildAt(i));
		if(CurrentItem !=nullptr)
		{
			CurrentItem->bIsSelected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}

void UUW_MainMenu::SetServerListItems(TArray<FServerData>newServerNames)
{
	UWorld* World = GetWorld();
	ServerList->ClearChildren();
	uint32 LocalIndex = 0;
	for(const FServerData& ServerData : newServerNames)
	{
		UW_ServerListItem* Item = CreateWidget<UW_ServerListItem>(World, ServerListItemClass);
		Item->ServerItem->SetText(FText::FromString(ServerData.ServerName));
		Item->HostName->SetText(FText::FromString(ServerData.HostName));
		Item->NumPlayers->SetText(FText::AsNumber(ServerData.CurrentPlayers));
		Item->MaxPlayers->SetText(FText::AsNumber(ServerData.MaxNumPlayers));
		Item->Setup(this, LocalIndex);
		ServerList->AddChild(Item);
		++LocalIndex;
	}
}

void UUW_MainMenu::SetSelectedIndex(uint32 newIndex)
{
	SelectedIndex = newIndex;
	UpdateServerListChildren();
}