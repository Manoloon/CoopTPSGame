// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PlayerAnimInstance.h"
#include "Entities/SCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	PawnCharacter = Cast<ASCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(PawnCharacter == nullptr)
	{
		PawnCharacter = Cast<ASCharacter>(TryGetPawnOwner());
	}
	if(PawnCharacter ==nullptr){return;}
	bDie = PawnCharacter->PlayerIsDead();
	if(!bDie)
	{
		bIsCrouched = PawnCharacter->bIsCrouched;
		bIsInAir = PawnCharacter->GetCharacterMovement()->IsFalling();
		FVector Velocity = PawnCharacter->GetVelocity();
		Velocity.Z = 0.f;
		Speed =  Velocity.Size();
		bIsAccelerating = (PawnCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()> 0.0f);
		bWeaponEquipped = PawnCharacter->IsWeaponEquipped();
		bIsAiming = PawnCharacter->IsAiming();
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity,PawnCharacter->GetActorRotation());
		//AimOffset
		AimOffsetData = PawnCharacter->GetAimOffsetData();
		// for the FABRIK -> TODO dont need to be in update
		LeftHandTransform = PawnCharacter->GetHandlingWeaponTransform();
		FVector OutPosition;
		FRotator OutRotation;
		PawnCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),
			LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));	
		//strafing
		const FRotator AimRotation = PawnCharacter->GetBaseAimRotation();
		const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PawnCharacter->GetVelocity());
		const FRotator LocalDeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,
		AimRotation);
		DeltaRotation=FMath::RInterpTo(DeltaRotation,LocalDeltaRot,DeltaSeconds,7.f);
		YawOffset = DeltaRotation.Yaw;
		// Lean
		CharacterRotationLastFrame = CharacterRotation;
		CharacterRotation = PawnCharacter->GetActorRotation();
		const FRotator DeltaActorRotation = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,
		CharacterRotationLastFrame);
		const float TargetRotation = DeltaActorRotation.Yaw /DeltaSeconds;
		const float InterpLean = FMath::FInterpTo(Lean,TargetRotation,DeltaSeconds,6.f);
		Lean = FMath::Clamp(InterpLean,-90.f,90.f);
	}
}
