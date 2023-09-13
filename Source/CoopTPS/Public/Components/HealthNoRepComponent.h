// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthNoRepComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPTPS_API UHealthNoRepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthNoRepComponent();
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float Health;
	bool bOwnerIsDead=false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;
	UFUNCTION()
	void DoDamage(float Damage);
	//** NetworkManager
	UFUNCTION()
	void PostReplication(TArray<uint8> Payload);
	TArray<uint8> Encode();
	void Decode(const TArray<uint8>& Payload);
	//***
	UPROPERTY()
	class ANetworkManager* NetworkManager;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
