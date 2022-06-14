// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RoleMessage.h"

void URoleMessage::SetDisplayText(const FString TextToDisplay) const
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void URoleMessage::ShowPlayerNetRole(const APawn* Pawn) const
{
	const ENetRole LocalRole = Pawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("NONE");
		break;
	default: ;
	}
	const FString LocalRoleString = FString::Printf(TEXT("Local Role %s"),*Role);
	SetDisplayText(LocalRoleString);
}

void URoleMessage::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
	RemoveFromParent();
}
