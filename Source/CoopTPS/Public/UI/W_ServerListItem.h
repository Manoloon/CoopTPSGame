// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_ServerListItem.generated.h"

/**
 * 
 */
UCLASS()
class COOPTPS_API UW_ServerListItem : public UUserWidget
{
	GENERATED_BODY()

public:
 	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ServerItem;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* NumPlayers;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* MaxPlayers;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HostName;

	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_ServerItem;

	void Setup(class UUW_MainMenu* newParent, uint32 newIndex);
	UPROPERTY(BlueprintReadOnly)
		bool bIsSelected = false;

private:

	UPROPERTY()
		class UUW_MainMenu* MyParent;

	uint32 MyIndex;

	UFUNCTION()
		void OnClickedItem();
	
};
