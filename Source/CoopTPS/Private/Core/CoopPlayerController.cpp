// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/CoopPlayerController.h"

#include "TextBlock.h"
#include "TPSHud.h"
#include "UI/UPlayerUI.h"
#include "UserWidget.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/IInputComm.h"

ACoopPlayerController::ACoopPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>HealthIndicatorClass(TEXT("/Game/UI/WB_HealthIndicator"));
	if(HealthIndicatorClass.Class)
	{
		HealthIndicator = HealthIndicatorClass.Class;
	}
}

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

void ACoopPlayerController::SetHUDScore(int Score)
{
	PlayerHUD = (PlayerHUD == nullptr)? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if(PlayerHUD && PlayerHUD->GetPlayerUI() && PlayerHUD->GetPlayerUI()->ScoreVal)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"),Score);
		PlayerHUD->GetPlayerUI()->ScoreVal->SetText(FText::FromString(ScoreText));
	}
}

void ACoopPlayerController::SetHUDHealth(float Health)
{
	PlayerHUD = (PlayerHUD == nullptr)? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if(PlayerHUD && PlayerHUD->GetHealthIndicator())
	{
		
	}
}

void ACoopPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ACoopPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	if(GetPawn() && APawn->Implements<UIInputComm>())
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
		PlayerHUD = (PlayerHUD==nullptr)? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
		if(PlayerHUD)
		{
			PlayerHUD->AddPlayerUI();
			SetHUDScore(GetPawn()->GetPlayerState()->GetScore());
			PlayerHUD->AddHealthIndicator();
			//SetHUDHealth()
		}
	}
}

void ACoopPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	if(HealthWidget)
	{
		HealthWidget->RemoveFromParent();	
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
}

void ACoopPlayerController::MoveForward(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveForward(Value);
	}
	else
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::MoveRight(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveRight(Value);
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
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::LookUpRate(float Value)
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_LookUpRate(Value);
	}
}

void ACoopPlayerController::StartAds()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartAiming();
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
		PawnInterface->I_StopAiming();
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
}

void ACoopPlayerController::StopFiring()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopFire();
	}
}

void ACoopPlayerController::Reload()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_Reload();
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
}

void ACoopPlayerController::StopThrow()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopThrow();
	}
}

void ACoopPlayerController::Jump()
{
	if(GetPawn() && PawnInterface)
	{
		PawnInterface->I_Jump();
	}
}
