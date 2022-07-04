// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HealthNoRepComponent.h"
//#include "Net/NetPushModelHelpers.h"
#include "GameFramework/Actor.h"
#include "NetworkManager.generated.h"

USTRUCT()
struct FHealthCompItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TArray<uint8> Data;
	FHealthCompItem();
	FHealthCompItem(const UHealthNoRepComponent* healthComponent, const TArray<uint8>& Payload);
	void PostReplicatedChange(const struct FHealthCompContainer& InArraySerializer);
};

USTRUCT()
struct FHealthCompContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY()
	TArray<FHealthCompItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FHealthCompItem,
													FHealthCompContainer>(Items,DeltaParams,*this);
	}
};

template<>
struct TStructOpsTypeTraits<FHealthCompContainer> : public TStructOpsTypeTraitsBase2<FHealthCompContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

inline FHealthCompItem::FHealthCompItem()
{
	OwnerActor = nullptr;
}

inline FHealthCompItem::FHealthCompItem(const UHealthNoRepComponent* HealthComponent, const 
TArray<uint8>& Payload)
{
	if (ensureAlways(HealthComponent))
	{
		OwnerActor = HealthComponent->GetOwner();
		Data = Payload;
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
inline void FHealthCompItem::PostReplicatedChange(const struct FHealthCompContainer& InArraySerializer)
{
	if (OwnerActor.Get())
	{
		if (UHealthNoRepComponent* Hc = OwnerActor.Get()->FindComponentByClass<UHealthNoRepComponent>())
		{
			Hc->PostReplication(Data);
		}
	}
}

UCLASS()
class ANetworkManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetworkManager();

	UPROPERTY(Replicated)
	FHealthCompContainer HealthTracker;

	UFUNCTION(BlueprintCallable)
	void RegisterActor(AActor* MyActor,const TArray<uint8>& Payload);
	UFUNCTION(BlueprintCallable)
	void UpdateActor(AActor* MyActor,const TArray<uint8>& Payload);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
