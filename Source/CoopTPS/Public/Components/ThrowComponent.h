// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThrowComponent.generated.h"
USTRUCT(BlueprintType)
struct FThrowData 
{
	GENERATED_USTRUCT_BODY()
		uint8 LaunchDistance = 100;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPTPS_API UThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UThrowComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	FVector Gravity = FVector(0.0f, 0.0f, -980.0f);
	TArray<UParticleSystemComponent*>BeamArray;


	void StarThrow();
	void StopThrow();
	void Throw();
	void ClearBeam();
	void AddNewBeam(FVector newPoint1, FVector newPoint2);
	void GetSegmentAtTime(FVector StartLocation, FVector InitialVelocity, FVector Gravity, float Time1, float Time2, FVector& OutPoint1, FVector& OutPoint2);
	void DrawingTrajectory();		
};
