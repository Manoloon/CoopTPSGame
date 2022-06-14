// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CoopGameInstance.h"
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
	Super::Init();
	if(const IOnlineSubsystem* CoopOnlineSubsystem = IOnlineSubsystem::Get(); CoopOnlineSubsystem !=nullptr)
	{
		OnlineSessionInterface = CoopOnlineSubsystem->GetSessionInterface();
		if(OnlineSessionInterface.IsValid())
		{
			OnlineSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
													&UCoopGameInstance::OnCreateSessionComplete);
			OnlineSessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, 
													&UCoopGameInstance::OnDestroySessionComplete);
			OnlineSessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, 
													&UCoopGameInstance::OnFindSessionsComplete);
			OnlineSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
			 										&UCoopGameInstance::OnJoinSessionComplete);
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

void UCoopGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UCoopGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
}

void UCoopGameInstance::I_Host(FString NewServerName)
{
	if (OnlineSessionInterface.IsValid()) 
	{ 
		DesiredServerName = NewServerName;
		if(const FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
			ExistingSession !=nullptr)
		{
			OnlineSessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}	
}

void UCoopGameInstance::I_Join(uint32 NewIndex)
{
	if (!OnlineSessionInterface.IsValid()) { return; }
	if (!OnlineSessionSearch.IsValid()) { return; }
	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}
	OnlineSessionInterface->JoinSession(0, SESSION_NAME, OnlineSessionSearch->SearchResults[NewIndex]);
}

void UCoopGameInstance::I_LoadMainMenu()
{
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel("/Game/Map/M_MainMenu", ETravelType::TRAVEL_Absolute);
	}
}

void UCoopGameInstance::I_QuitGame()
{
	if(APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		UEngine* Engine = GetEngine();
		if (!ensure(Engine != nullptr)) return;
		Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Quitting Game")));
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, true);
	}
}

void UCoopGameInstance::I_RefreshServerList()
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

			if (FString ServerName; Result.Session.SessionSettings.Get(SERVER_NAME_SETTING_KEY, ServerName))
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

	if (FString RemoteSession; !OnlineSessionInterface->GetResolvedConnectString(NewSessionName, RemoteSession))
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Remote Session"));
		return;
	}
	else
	{
		if (APlayerController* PlayerController = GetFirstLocalPlayerController(); PlayerController != nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("OnJoinSesionComplete"));
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
	// TODO : variable para cambiar de mapas segun session.
	UE_LOG(LogTemp, Error, TEXT("OnCreateSesionComplete"));
	GetWorld()->ServerTravel(FString("/Game/Map/M_Lobby?listen"));
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
	I_LoadMainMenu();
	//TODO : Mostrar cartel de error al usuario por desconexion.
}

//////////////// END CALLBACKS
void UCoopGameInstance::CreateSession() const
{
	FOnlineSessionSettings OnlineSessionSettings;
	// Si no usamos steam como subsystem , que sea  LAN PARTY
	UE_LOG(LogTemp,Warning,TEXT("Subsystem : %s"),*IOnlineSubsystem::Get()->GetSubsystemName().ToString());
	OnlineSessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	OnlineSessionSettings.NumPublicConnections = 4;
	
	OnlineSessionSettings.bAllowJoinInProgress = true;
	OnlineSessionSettings.bAllowJoinViaPresence = true;
	
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = true;
	OnlineSessionSettings.Set(SERVER_NAME_SETTING_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	OnlineSessionSettings.BuildUniqueId =1;
	OnlineSessionSettings.bUseLobbiesIfAvailable = true;

	OnlineSessionInterface->CreateSession(0, SESSION_NAME, OnlineSessionSettings);
}

void UCoopGameInstance::StartSession() const
{
	if(OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->StartSession(SESSION_NAME);
	}
}