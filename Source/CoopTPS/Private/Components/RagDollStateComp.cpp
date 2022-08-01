// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RagDollStateComp.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
URagDollStateComp::URagDollStateComp()
{
	PrimaryComponentTick.bStartWithTickEnabled=false;
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URagDollStateComp::BeginPlay()
{
	Super::BeginPlay();
	OwnerPawn = OwnerPawn ==nullptr ? Cast<ACharacter>(GetOwner()) : OwnerPawn;	
}

bool URagDollStateComp::PawnFacingUp() const
{
	if(OwnerPawn == nullptr){return false;}
	const FVector RightVectorFacing = UKismetMathLibrary::GetRightVector(OwnerPawn->GetMesh()->GetSocketRotation(FName
	("pelvis")));
	return UKismetMathLibrary::Dot_VectorVector(RightVectorFacing,FVector(0.f,0.f,1.f)) >= 0.0f;
}

void URagDollStateComp::CachePose()
{
	bIsFacingUp = PawnFacingUp();
	SetGetupOrientation();
	// select montage facing
	// delay 2 frames
	OwnerPawn->GetMesh()->GetAnimInstance()->SavePoseSnapshot(FName("DieSnap"));
	// Set OwnerDead;	
}

void URagDollStateComp::SetGetupOrientation() const
{
	if(OwnerPawn == nullptr){return;}
	const FVector NeckSocketLocation = OwnerPawn->GetMesh()->GetSocketLocation(FName("neck_01"));
	const FVector PelvisSocketLocation =OwnerPawn->GetMesh()->GetSocketLocation(FName("pelvis"));
	
	const FVector PointVector = bIsFacingUp ? FVector(PelvisSocketLocation - NeckSocketLocation) :
												FVector(NeckSocketLocation - PelvisSocketLocation);
	const FVector PawnLocation = OwnerPawn->GetMesh()->GetComponentLocation();
	const FRotator Rotation = UKismetMathLibrary::MakeRotFromZX(FVector{0.0,0.0,1.0},
							PointVector);
		const FTransform Result = FTransform(Rotation,PawnLocation,FVector(1.f));
		OwnerPawn->SetActorTransform(Result);
}

void URagDollStateComp::Activate(bool bReset)
{
	Super::Activate(bReset);
	OwnerPawn = Cast<ACharacter>(GetOwner());
}

UAnimMontage* URagDollStateComp::GetMontageToPlay() const
{
	return bIsFacingUp? FacingUpMontage : FacingDownMontage;
}

void URagDollStateComp::CalculateMeshLocation()
{
	if(OwnerPawn == nullptr){return;}
	FVector PelvisSocketLocation = OwnerPawn->GetMesh()->GetSocketLocation(FName("pelvis"));
	FVector OffsetTrace = FVector(0.f,0.f,-1.f) * 100.f; 
	FVector EndTrace = PelvisSocketLocation + OffsetTrace;
	FCollisionQueryParams QueryParams;
	if (FHitResult Hit; 
	GetWorld()->LineTraceSingleByChannel(Hit, PelvisSocketLocation, EndTrace,ECC_Visibility, QueryParams))
	{
		CapsuleLocation = Hit.Location + PelvisOffset;
	}
	else
	{
		CapsuleLocation = PelvisSocketLocation + PelvisOffset;
	}
	MeshLocation = UKismetMathLibrary::VInterpTo(MeshLocation,CapsuleLocation,30.f,
																	GetWorld()->GetDeltaSeconds());
}

void URagDollStateComp::ToggleRagDoll(const bool bOn)
{
	if(OwnerPawn == nullptr){return;}
	if(bOn)
	{
		OwnerPawn->GetMesh()->SetSimulatePhysics(true);
		bOwnerDead = true;
		OwnerPawn->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None,0);
		OwnerPawn->GetMesh()->SetConstraintProfileForAll(FName("ragdoll"),false);
	}
	else
	{
		CachePose();
		//Delay 0.2
		OwnerPawn->GetMesh()->SetSimulatePhysics(false);
		bOwnerDead = false;
		//Play montage
		//Revive pawn via animbp
	}
}

void URagDollStateComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(OwnerPawn)
	{
		if(bOwnerDead)
		{
			CalculateMeshLocation();
			OwnerPawn->GetCapsuleComponent()->SetWorldLocation(MeshLocation);
		}
		else
		{
			MeshLocation = OwnerPawn->GetMesh()->GetSocketLocation(FName("pelvis")) + PelvisOffset;
		}
	}
}

