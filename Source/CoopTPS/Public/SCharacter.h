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
class UParticleSystem;
class UDecalComponent;
class UMaterialInterface;
class USHealthComponent;

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
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UParticleSystem* BeamFX;
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UParticleSystemComponent* BeamComp;
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UDecalComponent* BeamEndPointDecal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HealthComp")
		USHealthComponent* HealthComp;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		bool bDied;
	
	// aiming variables
	bool bIsZoomed;
	float DefaultFOV;

	// grenade mode
	bool bIsGranadaMode;
	float LaunchDistance;
	FVector InitialVelocity;
	FVector InitialLocalVelocity;
	FVector StartLocation;
	FVector ThrowRotateVector;
	FVector SpawnScale;
	FRotator SpawnRotation;
	FVector Point1;
	FVector Point2;
	TArray<UParticleSystemComponent*>BeamArray;
	float PathLifeTime;
	float TimeInterval;
	FVector Gravity;
	UMaterialInterface* CurrentBeamEndPointMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Grenades")
	TSubclassOf<class ASProjectile> GranadaClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Grenades")
	FName GrenadeSocketName;

	// current weapon
	UPROPERTY(Replicated)
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

	void Reload();
	void Throw();
	void StartThrow();
	void StopThrow();
	void ClearBeam();
	void AddNewBeam(FVector Point1,FVector Point2);
	void GetSegmentAtTime(FVector StartLocation, FVector InitialVelocity, FVector Gravity, float Time1, float Time2, FVector &OutPoint1, FVector &OutPoint2);
	void DrawingTrajectory();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	// Esto esta expuesto para poder usarse en el BT.
	UFUNCTION(BlueprintCallable,Category = "Player")
	void StartFire();
	UFUNCTION(BlueprintCallable,Category = "Player")
	void StopFire();
};
