// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MainMenuInterface.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#include "CoopGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UCoopGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()
	
public:
	UCoopGameInstance(const FObjectInitializer& ObjectInitializer);

	void Init() override;
	UFUNCTION()
		void Host(FString NewServerName) override;
	UFUNCTION()
		void Join(uint32 newIndex) override;
	UFUNCTION()
		void LoadMainMenu() override;
	UFUNCTION()
		void QuitGame() override;
	UFUNCTION()
		void RefreshServerList() override;

	// son blueprint callables porque se los llama desde el level blueprint. 
	UFUNCTION(BlueprintCallable)
		void LoadMenu();
	UFUNCTION(BlueprintCallable)
		void LoadPauseMenu();

	void StartSession();

private:
	TSubclassOf<UUserWidget> MainMenuClass;
	class UUW_MainMenu* MainMenu;
	TSubclassOf<UUserWidget> PauseMenuClass;
	class UBaseMenuWidget* PauseMenu;

	// la interface de OnlineSession
	IOnlineSessionPtr OnlineSessionInterface;

	FString TravelURL;
	FString DesiredServerName;

	TSharedPtr<class FOnlineSessionSearch> OnlineSessionSearch;

	void OnCreateSessionComplete(FName newSessionName, bool Success);
	void OnDestroySessionComplete(FName newSessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName newSessionName, EOnJoinSessionCompleteResult::Type newResult);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	void CreateSession();


};
