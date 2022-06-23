// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ASCharacter;
UCLASS(Abstract)
class UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	UPROPERTY()
	ASCharacter* PawnCharacter =nullptr;
public:
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bIsCrouched=false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	float Speed =0.f;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	float YawOffset=0.f;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	float Lean=0.0f;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	float Direction =0.0f;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bIsInAir=false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bWeaponEquipped = false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bIsAccelerating=false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bIsAiming=false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	bool bDie =false;
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	FVector2D AimOffsetData={0.0f,0.f};
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly,Category ="Settings")
	FTransform LeftHandTransform;
	
	FRotator DeltaRotation;
	FRotator CharacterRotation;
	FRotator CharacterRotationLastFrame;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
