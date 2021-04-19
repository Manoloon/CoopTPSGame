// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopGameInstance.h"
#include "Uobject/ConstructorHelpers.h"
#include "UI/UW_MainMenu.h"
#include "UI/W_PauseMenu.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/BaseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

const static FName SESSION_NAME = TEXT("HORDE");

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
	IOnlineSubsystem* CoopOnlineSubsystem = IOnlineSubsystem::Get();
	if(CoopOnlineSubsystem !=nullptr)
	{
		OnlineSessionInterface = CoopOnlineSubsystem->GetSessionInterface();
		if(OnlineSessionInterface.IsValid())
		{
			OnlineSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCoopGameInstance::OnCreateSessionComplete);
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCoopGameInstance::OnDestroySessionComplete);
			OnlineSessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCoopGameInstance::OnFindSessionsComplete);
			OnlineSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCoopGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Online Subsystem NOT FOUND"));
	}
}

void UCoopGameInstance::Host()
{
	if (OnlineSessionInterface.IsValid()) 
	{ 
		FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(SESSION_NAME);
		if(ExistingSession !=nullptr)
		{
			OnlineSessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
	
}

void UCoopGameInstance::Join(uint32 newIndex)
{
	if (!OnlineSessionInterface.IsValid()) { return; }
	if (!OnlineSessionSearch.IsValid()) { return; }
	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}
	OnlineSessionInterface->JoinSession(0, SESSION_NAME, OnlineSessionSearch->SearchResults[newIndex]);
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

void UCoopGameInstance::RefreshServerList()
{
	if (!OnlineSessionInterface.IsValid()) { return; }
	OnlineSessionSearch = MakeShareable(new FOnlineSessionSearch);

	if(OnlineSessionSearch.IsValid())
	{
		// mientras el SteamID sea el 480 = osea gratuito de prueba. Luego podra ser 1 
		OnlineSessionSearch->MaxSearchResults = 100;
		OnlineSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		OnlineSessionInterface->FindSessions(0, OnlineSessionSearch.ToSharedRef());
		UE_LOG(LogTemp, Warning, TEXT("Start Session Found"));
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

// ONLINE SUBSYSTEM CALLBACKS!

void UCoopGameInstance::OnCreateSessionComplete(FName newSessionName, bool Success)
{
	if(!Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create session"));
		return;
	}
	if(MainMenu !=nullptr)
	{
		MainMenu->Teardown();
	}
	UWorld* World = GetWorld();
	// TODO : variable para cambiar de mapas segun session. 
	World->ServerTravel("/Game/Map/M_Level1?listen");
}

void UCoopGameInstance::OnDestroySessionComplete(FName newSessionName, bool Success)
{
	if(Success)
	{
		CreateSession();
	}
}

void UCoopGameInstance::OnFindSessionsComplete(bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Finish Session Found"));
	if(Success && OnlineSessionSearch.IsValid() && MainMenu !=nullptr)
	{
		TArray<FOnlineSessionSearchResult>Results = OnlineSessionSearch->SearchResults;
		TArray<FString>ServerSessionNames;
		for(FOnlineSessionSearchResult& Result : Results)
		{
			ServerSessionNames.Add(Result.GetSessionIdStr());
		}
		MainMenu->SetServerListItems(ServerSessionNames);
	}
}

void UCoopGameInstance::OnJoinSessionComplete(FName newSessionName, EOnJoinSessionCompleteResult::Type newResult)
{
	if (!OnlineSessionInterface.IsValid()) { return; }

	FString RemoteSession;
	if(!OnlineSessionInterface->GetResolvedConnectString(newSessionName,RemoteSession))
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Remote Session"));
		return;
	}
	else
	{
		APlayerController* PlayerController = GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(RemoteSession, ETravelType::TRAVEL_Absolute);
	 	}
	}
}

void UCoopGameInstance::CreateSession()
{
	FOnlineSessionSettings OnlineSessionSettings;
	OnlineSessionSettings.bIsLANMatch = false;
	OnlineSessionSettings.NumPublicConnections = 4;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = true;
	OnlineSessionInterface->CreateSession(0, SESSION_NAME, OnlineSessionSettings);
}
