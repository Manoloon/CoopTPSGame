// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/CoopPlayerController.h"
#include "Core/SGameMode.h"
#include "Components/TextBlock.h"
#include "Core/TPSHud.h"
#include "UI/UPlayerUI.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/IInputComm.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputConfigData.h"
#include "Kismet/GameplayStatics.h"

void ACoopPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(InputActions->InputMove,ETriggerEvent::Triggered,this,&ACoopPlayerController::Move);
		EnhancedInputComponent->BindAction(InputActions->InputLook,ETriggerEvent::Triggered,this,&ACoopPlayerController::Look);

		EnhancedInputComponent->BindAction(InputActions->InputJump,ETriggerEvent::Triggered,this,&ACoopPlayerController::Jump);
		
		EnhancedInputComponent->BindAction(InputActions->InputCrouch,ETriggerEvent::Triggered,this,&ACoopPlayerController::StartCrouch);
		EnhancedInputComponent->BindAction(InputActions->InputCrouch,ETriggerEvent::Completed,this,&ACoopPlayerController::StopCrouch);

		EnhancedInputComponent->BindAction(InputActions->InputADS,ETriggerEvent::Triggered,this,&ACoopPlayerController::StartAds);
		EnhancedInputComponent->BindAction(InputActions->InputADS,ETriggerEvent::Canceled,this,&ACoopPlayerController::StopAds);

		EnhancedInputComponent->BindAction(InputActions->InputReload,ETriggerEvent::Triggered,this,&ACoopPlayerController::Reload);

		EnhancedInputComponent->BindAction(InputActions->InputThrow,ETriggerEvent::Triggered,this,&ACoopPlayerController::StartThrow);
		EnhancedInputComponent->BindAction(InputActions->InputThrow,ETriggerEvent::Canceled,this,&ACoopPlayerController::StopThrow);

		EnhancedInputComponent->BindAction(InputActions->InputInteract,ETriggerEvent::Triggered,this,&ACoopPlayerController::Interact);

		EnhancedInputComponent->BindAction(InputActions->InputPrimaryFire,ETriggerEvent::Triggered,this,&ACoopPlayerController::StartFiring);
		EnhancedInputComponent->BindAction(InputActions->InputPrimaryFire,ETriggerEvent::Canceled,this,&ACoopPlayerController::StopFiring);

		EnhancedInputComponent->BindAction(InputActions->InputChangeWeapon,ETriggerEvent::Triggered,this,&ACoopPlayerController::ChangeWeapon);
	}
}

void ACoopPlayerController::SetHudScore(const int Score)
{
	PlayerHUD = (PlayerHUD == nullptr) ? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->GetPlayerUI() && PlayerHUD->GetPlayerUI()->ScoreVal)
	{
		const FString ScoreText = FString::Printf(TEXT("%02d"), Score);
		PlayerHUD->GetPlayerUI()->ScoreVal->SetText(FText::FromString(ScoreText));
	}
}

void ACoopPlayerController::SetHudGameTime()
{
	PlayerHUD = (PlayerHUD == nullptr) ? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->GetPlayerUI())
	{
		const uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
		if (Countdown != SecondsLeft)
		{
			// Set hud time
			const float CountDownTime = MatchTime - GetServerTime();
			const int32 Minutes = FMath::FloorToInt(CountDownTime / 60);
			const int32 Seconds = CountDownTime - Minutes * 60;

			const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
			PlayerHUD->GetPlayerUI()->SetMatchTime(CountdownText);
		}
		Countdown = SecondsLeft;
	}
}

void ACoopPlayerController::UpdateCurrentAmmo(const int32 NewAmmo, const int32 NewMaxAmmo)
{
	PlayerHUD = (PlayerHUD == nullptr) ? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->GetPlayerUI())
	{
		PlayerHUD->GetPlayerUI()->SetCurrentAmmo(NewAmmo, NewMaxAmmo);
	}
}

void ACoopPlayerController::SetWeaponInfoHUD(const FName NewWeaponName, const int32 NewCurrentAmmo, const int32 NewMaxAmmo)
{
	PlayerHUD = (PlayerHUD == nullptr) ? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->GetPlayerUI())
	{
		PlayerHUD->GetPlayerUI()->SetWeaponName(NewWeaponName);
		PlayerHUD->GetPlayerUI()->SetCurrentAmmo(NewCurrentAmmo, NewMaxAmmo);
	}
}

void ACoopPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (FTimerHandle Th_MatchTime;
		!GetWorldTimerManager().IsTimerActive(Th_MatchTime))
	{
		GetWorldTimerManager().SetTimer(Th_MatchTime, this, &ACoopPlayerController::SetHudGameTime, 0.5f, true);
	}
}

void ACoopPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ACoopPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	if (GetPawn() && APawn->Implements<UIInputComm>())
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
	PlayerHUD = (PlayerHUD == nullptr) ? Cast<ATPSHud>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->GetPlayerUI())
	{
		SetHudScore(GetPawn()->GetPlayerState()->GetScore());
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(this->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMapping, 0);
	}
}

void ACoopPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	if (const auto GameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->RespawnPlayer(this);
	}
}

void ACoopPlayerController::Server_RequestServerTime_Implementation(const float TimeClientRequest)
{
	const float ServerTimeReceipt = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeClientRequest, ServerTimeReceipt);
}

void ACoopPlayerController::Client_ReportServerTime_Implementation(const float TimeClientRequest,
	const float TimeServerReceiveClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeClientRequest;
	HalfRoundtripTime = (0.5f * RoundTripTime);
	const float CurrentServerTime = TimeServerReceiveClientRequest + HalfRoundtripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ACoopPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ACoopPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		if (FTimerHandle Th_SyncTime;
			!GetWorldTimerManager().IsTimerActive(Th_SyncTime))
		{
			GetWorldTimerManager().SetTimer(Th_SyncTime, this, &ACoopPlayerController::SyncTime,
				TimeSyncFrequency, true);
		}
	}
}

void ACoopPlayerController::SyncTime()
{
	// get ping
	const float CurrentPing = PlayerState->GetPingInMilliseconds();
	// UE_LOG(LogTemp,Warning,TEXT("Ping is %f"),CurrentPing);
	if (PlayerHUD && CurrentPing)
	{
		PlayerHUD->GetPlayerUI()->SetPingIndicator(CurrentPing);
	}
	Server_RequestServerTime(GetWorld()->GetTimeSeconds());
}

/*INPUTS */
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StartRun()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartRun();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StopRun()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopRun();
	}
}

void ACoopPlayerController::MoveForward(float Value)
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveForward(Value);
	}
	else
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(GetPawn()) && PawnInterface)
	{
		PawnInterface->I_MoveForward(MovementVector.Y);
		PawnInterface->I_MoveRight(MovementVector.X);
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (IsValid(GetPawn()) && PawnInterface)
	{
		// add yaw and pitch input to controller
		PawnInterface->I_TurnRate(LookAxisVector.X);
		PawnInterface->I_LookUpRate(-LookAxisVector.Y);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::MoveRight(float Value)
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_MoveRight(Value);
	}
}

void ACoopPlayerController::TurnRate(float Value)
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_TurnRate(Value);
	}
	else
	{
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::LookUpRate(float Value)
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_LookUpRate(Value);
	}
}

void ACoopPlayerController::StartAds()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartAiming();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}

void ACoopPlayerController::StopAds()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopAiming();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("fail to get my pawn!"));
		PawnInterface = Cast<IIInputComm>(GetPawn());
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StartCrouch()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartCrouch();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StopCrouch()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopCrouch();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StartFiring()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartFire();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StopFiring()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopFire();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::Reload()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_Reload();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StartThrow()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StartThrow();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::StopThrow()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_StopThrow();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::Jump()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_Jump();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::Interact()
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_PickupWeapon();
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst
void ACoopPlayerController::ChangeWeapon(const FInputActionValue& Value)
{
	if (GetPawn() && PawnInterface)
	{
		PawnInterface->I_SwapWeapons();
	}
}
