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
		void Host();
	UFUNCTION()
		void Join(const FString& Address);
	UFUNCTION(BlueprintCallable)
		void LoadMenu();

private:
	TSubclassOf<UUserWidget> MainMenuClass;
	class UUW_MainMenu* MainMenu;
};
