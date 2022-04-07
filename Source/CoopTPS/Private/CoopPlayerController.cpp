// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopPlayerController.h"
#include "Interfaces/IInputComm.h"

void ACoopPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward",this,&ACoopPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this,&ACoopPlayerController::MoveRight);
	
	InputComponent->BindAxis("LookUp", this, &ACoopPlayerController::LookUpRate);
	InputComponent->BindAxis("Turn", this, &ACoopPlayerController::TurnRate);
	
	InputComponent->BindAction("Crouch", IE_Pressed, this, &ACoopPlayerController::StartCrouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &ACoopPlayerController::StopCrouch);

	InputComponent->BindAction("Jump", IE_Pressed, this,&ACoopPlayerController::Jump);

	InputComponent->BindAction("ADS", IE_Pressed, this, &ACoopPlayerController::StartAds);
	InputComponent->BindAction("ADS", IE_Released, this, &ACoopPlayerController::StopAds);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ACoopPlayerController::StartFiring);
	InputComponent->BindAction("Fire", IE_Released, this, &ACoopPlayerController::StopFiring);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ACoopPlayerController::Reload);
	
	InputComponent->BindAction("Throw", IE_Pressed, this, &ACoopPlayerController::StartThrow);
	InputComponent->BindAction("Throw", IE_Released, this, &ACoopPlayerController::StopThrow);

	InputComponent->BindAction("ChangeWeapon", IE_Released, this, &ACoopPlayerController::ChangeWeapon);
}

void ACoopPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	if(APawn->Implements<UIInputComm>())
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StartRun()
{
	if(PawnInterface)
	{
		PawnInterface->I_StartRun();
	}
}

void ACoopPlayerController::StopRun()
{
	if(PawnInterface)
	{
		PawnInterface->I_StopRun();
	}
}

void ACoopPlayerController::MoveForward(float Value)
{
	if(PawnInterface)
	{
		PawnInterface->I_MoveForward(Value);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::MoveRight(float Value)
{
	if(PawnInterface)
	{
		PawnInterface->I_MoveRight(Value);
	}
}

void ACoopPlayerController::TurnRate(float Value)
{
	if(PawnInterface)
	{
		PawnInterface->I_TurnRate(Value);
	}
}

void ACoopPlayerController::LookUpRate(float Value)
{
	if(PawnInterface)
	{
		PawnInterface->I_LookUpRate(Value);
	}
}

void ACoopPlayerController::StartAds()
{
	if(PawnInterface)
	{
		PawnInterface->I_StartADS();
	}
}

void ACoopPlayerController::StopAds()
{
	if(PawnInterface)
	{
		PawnInterface->I_StopADS();
	}
}

void ACoopPlayerController::StartCrouch()
{
	if(PawnInterface)
	{
		PawnInterface->I_StartCrouch();
	}
}

void ACoopPlayerController::StopCrouch()
{
	if(PawnInterface)
	{
		PawnInterface->I_StopCrouch();
	}
}

void ACoopPlayerController::StartFiring()
{
	if(PawnInterface)
	{
		PawnInterface->I_StartFire();
	}
}

void ACoopPlayerController::StopFiring()
{
	if(PawnInterface)
	{
		PawnInterface->I_StopFire();
	}
}

void ACoopPlayerController::Reload()
{
	if(PawnInterface)
	{
		PawnInterface->I_Reload();
	}
}

void ACoopPlayerController::ChangeWeapon()
{
	if(PawnInterface)
	{
		PawnInterface->I_ChangeWeapon();
	}
}

void ACoopPlayerController::StartThrow()
{
	if(PawnInterface)
	{
		PawnInterface->I_StartThrow();
	}
}

void ACoopPlayerController::StopThrow()
{
	if(PawnInterface)
	{
		PawnInterface->I_StopThrow();
	}
}

void ACoopPlayerController::Jump()
{
	if(PawnInterface)
	{
		PawnInterface->I_Jump();
	}
}
