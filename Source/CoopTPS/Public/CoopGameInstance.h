// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CoopGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UCoopGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCoopGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init();
	UFUNCTION(Exec)
		void Host();
	UFUNCTION(Exec)
		void Join(const FString& Address);
	UFUNCTION(BlueprintCallable)
		void LoadMenu();

private:
	TSubclassOf<UUserWidget> MainMenuClass;

};
