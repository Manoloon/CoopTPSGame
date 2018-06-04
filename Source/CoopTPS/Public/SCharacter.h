// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;
class ASWeapon;
class ASProjetile;

UCLASS()
class COOPTPS_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComp")
		UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComp")
		USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ThrowPosition")
	USceneComponent* ThrowPosition;

	// aiming variables
	bool bIsZoomed;
	float DefaultFOV;

	// grenade mode
	bool bIsGranadaMode;
	FVector ThrowOffset;
	float LaunchDistance;
	FVector InitialVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Grenades")
	TSubclassOf<class ASProjectile> GranadaClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Grenades")
	FName GrenadeSocketName;

	// current weapon
	ASWeapon* CurrentWeapon;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<ASWeapon>StarterWeaponClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName WeaponSocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = 10.0,ClampMax = 40.0))
		float ZoomInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float ZoomedFOV;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();
	void BeginZoom();
	void EndZoom();
	void StartFire();
	void StopFire();
	void Reload();
	void Throw();
	void StartThrow();
	void StopThrow();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;
	
};
