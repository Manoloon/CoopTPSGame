// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define SURFACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2

#define COLLISION_WEAPON			ECC_GameTraceChannel1
#define COLLISION_PAWN				ECC_GameTraceChannel2

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