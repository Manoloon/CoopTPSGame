// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/SPlayerState.h"

void ASPlayerState::AddScore(const float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);
}
