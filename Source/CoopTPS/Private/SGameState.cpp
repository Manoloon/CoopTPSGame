// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameState.h"
#include "Net/UnrealNetwork.h"



void ASGameState::OnRep_WaveState(EWaveState OldWaveState)
{
	WaveStateChanged(WaveState,OldWaveState);
}

// networking
void ASGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME(ASGameState, WaveState);

}