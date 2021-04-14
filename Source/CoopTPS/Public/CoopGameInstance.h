// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MainMenuInterface.h"
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
		virtual void Join(const FString& Address) override;
	UFUNCTION()
		virtual void LoadMainMenu() override;
	UFUNCTION()
		virtual void QuitGame() override;

	// son blueprint callables porque se los llama desde el level blueprint. 
	UFUNCTION(BlueprintCallable)
		void LoadMenu();
	UFUNCTION(BlueprintCallable)
		void LoadPauseMenu();


private:
	TSubclassOf<UUserWidget> MainMenuClass;
	class UBaseMenuWidget* MainMenu;
	TSubclassOf<UUserWidget> PauseMenuClass;
	UBaseMenuWidget* PauseMenu;
};
