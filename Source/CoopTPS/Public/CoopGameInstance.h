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

	virtual void Init();
	UFUNCTION()
		virtual void Host() override;
	UFUNCTION()
		virtual void Join(uint32 newIndex) override;
	UFUNCTION()
		virtual void LoadMainMenu() override;
	UFUNCTION()
		virtual void QuitGame() override;
	UFUNCTION()
		void RefreshServerList() override;

	// son blueprint callables porque se los llama desde el level blueprint. 
	UFUNCTION(BlueprintCallable)
		void LoadMenu();
	UFUNCTION(BlueprintCallable)
		void LoadPauseMenu();


private:
	TSubclassOf<UUserWidget> MainMenuClass;
	class UUW_MainMenu* MainMenu;
	TSubclassOf<UUserWidget> PauseMenuClass;
	class UBaseMenuWidget* PauseMenu;

	// la interface de OnlineSession
	IOnlineSessionPtr OnlineSessionInterface;

	TSharedPtr<class FOnlineSessionSearch> OnlineSessionSearch;

	void OnCreateSessionComplete(FName newSessionName, bool Success);
	void OnDestroySessionComplete(FName newSessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName newSessionName, EOnJoinSessionCompleteResult::Type newResult);

	void CreateSession();

};
