// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/W_ServerListItem.h"
#include "UW_MainMenu.h"
#include "Components/Button.h"

void UW_ServerListItem::Setup(class UUW_MainMenu* newParent, uint32 newIndex)
{
	MyParent = newParent;
	MyIndex = newIndex;
	if (BTN_ServerItem == nullptr) { return; }
	BTN_ServerItem->OnClicked.AddDynamic(this, &UW_ServerListItem::OnClickedItem);
}

void UW_ServerListItem::OnClickedItem()
{
	MyParent->SetSelectedIndex(MyIndex);
}
