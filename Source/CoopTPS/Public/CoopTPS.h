// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define SURFACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2

#define COLLISION_WEAPON			ECC_GameTraceChannel1
#define COLLISION_PAWN				ECC_GameTraceChannel2

UENUM(BlueprintType)
enum class EGSAbilityInputID : uint8
{
	// 0 None
	None				UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm				UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel				UMETA(DisplayName = "Cancel"),
	// 3 ADS
	PrimaryFire			UMETA(DisplayName = "Primary Fire")
};

#if !IS_PROGRAM && !UE_BUILD_SHIPPING && (PLATFORM_WINDOWS || PLATFORM_PS4 || PLATFORM_XBOXONE)
#define LOGTRACE_ENABLED 1
#else
#define LOGTRACE_ENABLED 0
#endif

// whether the platform will signal a controller pairing change on a controller disconnect.
// if not, we need to treat the pairing change as a request to switch profiles when
// the destination profile is not specified
#ifndef OCC_CONTROLLER_PAIRING_ON_DISCONNECT
#define OCC_CONTROLLER_PAIRING_ON_DISCONNECT 1
#endif

// whether the game should display an account picker when a new input device is connected, while the "please reconnect controller" message is on screen.
#ifndef OCC_CONTROLLER_PAIRING_PROMPT_FOR_NEW_USER_WHEN_RECONNECTING
#define OCC_CONTROLLER_PAIRING_PROMPT_FOR_NEW_USER_WHEN_RECONNECTING 0
#endif