// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CoopAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName,PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName,PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class COOPTPS_API UCoopAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCoopAttributeSet();
	
	UPROPERTY(BlueprintReadOnly,Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCoopAttributeSet,Health);

	UPROPERTY(BlueprintReadOnly,Category = "Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UCoopAttributeSet,Armor);
};
