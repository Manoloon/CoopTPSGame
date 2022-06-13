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

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;
	UFUNCTION()
		virtual void Host(FString NewServerName) override;
	UFUNCTION()
		virtual void Join(uint32 NewIndex) override;
	UFUNCTION()
		virtual void LoadMainMenu() override;
	UFUNCTION()
		virtual void QuitGame() override;
	UFUNCTION()
		virtual void RefreshServerList() override;

	// son blueprint callables porque se los llama desde el level blueprint. 
	UFUNCTION(BlueprintCallable)
		void LoadMenu();
	UFUNCTION(BlueprintCallable)
		void LoadPauseMenu();

	void StartSession() const;

private:
	TSubclassOf<UUserWidget> MainMenuClass;
	UPROPERTY()
	class UUW_MainMenu* MainMenu;
	TSubclassOf<UUserWidget> PauseMenuClass;
	UPROPERTY()
	class UBaseMenuWidget* PauseMenu;

	// la interface de OnlineSession
	IOnlineSessionPtr OnlineSessionInterface;

	FString TravelURL;
	FString DesiredServerName;

	TSharedPtr<class FOnlineSessionSearch> OnlineSessionSearch;

	void OnCreateSessionComplete(FName NewSessionName, bool Success);
	void OnDestroySessionComplete(FName NewSessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName NewSessionName, EOnJoinSessionCompleteResult::Type NewResult);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	void CreateSession() const;


};
