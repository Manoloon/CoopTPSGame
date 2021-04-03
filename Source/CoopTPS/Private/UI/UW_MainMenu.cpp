// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_MainMenu.h"
#include "Components/Button.h"

bool UUW_MainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { return false; }
	BTN_Host->OnClicked.AddDynamic(this, &UUW_MainMenu::HostServer);
	return true;
}

void UUW_MainMenu::HostServer()
{

}