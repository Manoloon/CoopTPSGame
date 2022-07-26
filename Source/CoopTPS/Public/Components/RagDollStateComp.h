// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "RagDollStateComp.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPTPS_API URagDollStateComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	URagDollStateComp();
	UAnimMontage* GetMontageToPlay() const;
	//void SetMeshLocation(FVector NewMeshLocation);
	void CalculateMeshLocation();
	void ToggleRagDoll(const bool bOn);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Activate(bool bReset) override;
protected:
	UPROPERTY()
	ACharacter* OwnerPawn=nullptr;
	bool bOwnerDead=false;
	bool bIsFacingUp=false;
	FVector MeshLocation;
	FVector CapsuleLocation;
	UPROPERTY(EditAnywhere,Category="Settings")
		UAnimMontage* FacingUpMontage;
	UPROPERTY(EditAnywhere,Category="Settings")
		UAnimMontage* FacingDownMontage;
	UPROPERTY(EditAnywhere,Category = "Settings")
		FVector PelvisOffset = FVector(0.f,0.f,97.f);
	virtual void BeginPlay() override;
	bool PawnFacingUp() const;
	void CachePose();
	void SetGetupOrientation() const;
};
