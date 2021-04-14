// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MainMenuInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMainMenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COOPTPS_API IMainMenuInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// para que la funcion virtual funcione , tiene que llamarse igual que la implementacion en el game instance , por ejemplo. 
	virtual void Host() = 0;
	virtual void Join(const FString& Address) = 0;
	virtual void LoadMainMenu() = 0;
	virtual void QuitGame() = 0;
};
