// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CoopPlayerController.h"
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
	if(GetPawn() && APawn->Implements<UIInputComm>())
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StartRun()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartRun();
	}
}

void ACoopPlayerController::StopRun()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopRun();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::MoveForward(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveForward(Value);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::MoveRight(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveRight(Value);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::TurnRate(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_TurnRate(Value);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::LookUpRate(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_LookUpRate(Value);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StartAds()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartADS();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StopAds()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopADS();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StartCrouch()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartCrouch();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StopCrouch()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopCrouch();
	}
}

void ACoopPlayerController::StartFiring()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartFire();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StopFiring()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopFire();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::Reload()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_Reload();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::ChangeWeapon()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_ChangeWeapon();
	}
}

void ACoopPlayerController::StartThrow()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartThrow();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StopThrow()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopThrow();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::Jump()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_Jump();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}
