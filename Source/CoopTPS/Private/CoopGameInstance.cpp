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

const static FName SESSION_NAME = TEXT("GameSession");
const static FName SERVER_NAME_SETTING_KEY = TEXT("Server Name");

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
	if(GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UCoopGameInstance::OnNetworkFailure);
	}
}

void UCoopGameInstance::Host(FString NewServerName)
{
	if (OnlineSessionInterface.IsValid()) 
	{ 
		DesiredServerName = NewServerName;
		FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
		if(ExistingSession !=nullptr)
		{
			OnlineSessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}	
}

void UCoopGameInstance::Join(uint32 NewIndex)
{
	if (!OnlineSessionInterface.IsValid()) { return; }
	if (!OnlineSessionSearch.IsValid()) { return; }
	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}
	OnlineSessionInterface->JoinSession(0, SESSION_NAME, OnlineSessionSearch->SearchResults[NewIndex]);
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

void UCoopGameInstance::OnFindSessionsComplete(const bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Finish Session Found"));
	if (Success && OnlineSessionSearch.IsValid() && MainMenu != nullptr)
	{
		TArray<FOnlineSessionSearchResult>Results = OnlineSessionSearch->SearchResults;
		TArray<FServerData>ServerSessionNames;
		for (FOnlineSessionSearchResult& Result : Results)
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Found %s"), *Result.GetSessionIdStr());
			FServerData LocalData;

			LocalData.HostName = Result.Session.OwningUserName;
			LocalData.MaxNumPlayers = Result.Session.SessionSettings.NumPublicConnections;
			LocalData.CurrentPlayers = (LocalData.MaxNumPlayers - Result.Session.NumOpenPublicConnections);

			FString ServerName;
			if (Result.Session.SessionSettings.Get(SERVER_NAME_SETTING_KEY, ServerName))
			{
				LocalData.ServerName = ServerName;
			}
			else
			{
				LocalData.ServerName = "Could not found name";
			}
			ServerSessionNames.Add(LocalData);
		}
		MainMenu->SetServerListItems(ServerSessionNames);
	}
}

void UCoopGameInstance::OnJoinSessionComplete(const FName NewSessionName, EOnJoinSessionCompleteResult::Type NewResult)
{
	if (!OnlineSessionInterface.IsValid()) { return; }

	FString RemoteSession;
	if (!OnlineSessionInterface->GetResolvedConnectString(NewSessionName, RemoteSession))
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Remote Session"));
		return;
	}
	else
	{
		APlayerController* PlayerController = GetFirstLocalPlayerController();
		if (PlayerController != nullptr)
		{
			PlayerController->ClientTravel(RemoteSession, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UCoopGameInstance::OnCreateSessionComplete(FName NewSessionName, const bool Success)
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
	World->ServerTravel("/Game/Map/M_Lobby?listen");
}

void UCoopGameInstance::OnDestroySessionComplete(FName NewSessionName, const bool Success)
{
	if(Success)
	{
		CreateSession();
	}
}

void UCoopGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	LoadMainMenu();
	//TODO : Mostrar cartel de error al usuario por desconexion.
}

//////////////// END CALLBACKS



void UCoopGameInstance::CreateSession() const
{
	FOnlineSessionSettings OnlineSessionSettings;
	// Si no usamos steam como subsystem , que sea  LAN PARTY
	OnlineSessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");

	OnlineSessionSettings.NumPublicConnections = 4;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = true;
	OnlineSessionSettings.Set(SERVER_NAME_SETTING_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionInterface->CreateSession(0, SESSION_NAME, OnlineSessionSettings);
}

void UCoopGameInstance::StartSession() const
{
	if(OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->StartSession(SESSION_NAME);
	}
}
