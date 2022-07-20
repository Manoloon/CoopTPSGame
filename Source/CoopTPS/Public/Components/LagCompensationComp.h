// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCharacter.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComp.generated.h"

USTRUCT(BlueprintType)
struct FServerSideRewindResults
{
	GENERATED_BODY()
	UPROPERTY()
	bool bHitConfirmed;
	UPROPERTY()
	bool bHeadShotConfirmed;
};

USTRUCT(BlueprintType)
struct FHitBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;
	UPROPERTY()
	TMap<FName,FHitBoxInfo> HitBoxInfoContainer;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULagCompensationComp : public UActorComponent
{
	GENERATED_BODY()
	TDoubleLinkedList<FFramePackage> FrameHistory;
	float MaxRecordedTime = 4.f;
public:	
	ULagCompensationComp();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(AActor* HitCharacter, const FVector_NetQuantize& StartTrace, const 
	                        FVector_NetQuantize& HitLocation, float HitTime, AActor* DamageCauser, float DamageValue, TSubclassOf<UDamageType>
	                        DamageType);
	FServerSideRewindResults ServerSideRewind(class ASCharacter* HitCharacter,
	                                          const FVector_NetQuantize& TraceStart,
	                                          const FVector_NetQuantize& HitLocation,
	                                          float HitTime) const;
protected:
	virtual void BeginPlay() override;
	void ShowFramePackage(const FFramePackage& FramePackage) const;
	void SaveFramePackage(FFramePackage& NewFramePackage) const;
	void SaveFramePackage();
	static void MoveCollisionBoxes(const ASCharacter* HitCharacter,const FFramePackage& Package);
	static void ResetHitBoxes(const ASCharacter* HitCharacter,const FFramePackage& Package);
	static void EnablePawnMeshCollision(const ASCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable);
	static void CacheBoxPosition(const ASCharacter* HitCharacter, FFramePackage& OutFramePackage);
	FServerSideRewindResults ConfirmHit(const FFramePackage& Package,
										ASCharacter* HitCharacter,
										const FVector_NetQuantize& StartTrace,
										const FVector_NetQuantize& HitLocation) const;
	FFramePackage FramesInterpolation(const FFramePackage& OlderFrame,
										const FFramePackage& YoungerFrame,
										float HitTime) const;
};
