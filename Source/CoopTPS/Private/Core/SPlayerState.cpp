// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"

void ASPlayerState::AddScore(const float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);
}
