// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IHealthyActor.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInputComm.h"
#include "Interfaces/IHealthyActor.h"
#include "SCharacter.generated.h"

// USTRUCT()
// struct FWeaponsItems
// {
// 	GENERATED_USTRUCT_BODY()
// 
// 	AActor* WeaponToSpawn;
// 	class UTexture2D WeaponImage;
// };
UCLASS(Abstract)
class COOPTPS_API ASCharacter final : public ACharacter, public IIInputComm, public IIHealthyActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComp")
		class UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComp")
		class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UParticleSystem* BeamFX;
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UParticleSystemComponent* BeamComp;
	UPROPERTY(EditDefaultsOnly, Category = "GrenadeMode")
		UDecalComponent* BeamEndPointDecal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HealthComp")
		class USHealthComponent* HealthComp;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		bool bDied = false;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
		USkeletalMesh* PawnMesh = nullptr;

	// aiming variables
	bool bIsZoomed;
	float DefaultFOV;

	// grenade mode
	//UPROPERTY()
	//class ASProjectile* Grenade = nullptr;
	bool bIsGranadaMode = false;
	float LaunchDistance = 100.0f;
	FVector InitialVelocity;
	FVector InitialLocalVelocity;
	FVector StartLocation;
	FVector ThrowRotateVector;
	FVector SpawnScale = FVector(1.0f);
	FRotator SpawnRotation;
	FVector Point1;
	FVector Point2;
	TArray<UParticleSystemComponent*>BeamArray;
	float PathLifeTime = 5.0f;
	float TimeInterval = 0.05;
	FVector Gravity = FVector(0.0f, 0.0f, -980.0f);
	UPROPERTY()
		UMaterialInterface* CurrentBeamEndPointMaterial;

	// material dinamico para el color del player
	UPROPERTY(BlueprintReadWrite)
		class UMaterialInstanceDynamic* MeshID;

	UPROPERTY(EditDefaultsOnly, Category = "Grenades")
	TSubclassOf<class ASProjectile> GranadaClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Grenades")
	FName GrenadeSocketName = "GrenadeSocket";

	// current weapon
public:
	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		class ASWeapon* CurrentWeapon= nullptr;
	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		ASWeapon* PrimaryWeapon = nullptr;
	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		ASWeapon* SecondaryWeapon = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerColor,EditAnywhere,BlueprintReadOnly,Transient)
	FLinearColor PlayerColor;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<ASWeapon>StarterWeaponClass;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<ASWeapon>SecondaryWeaponClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName WeaponSocketName = "WeaponSocket";
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName SecondaryWeaponSocketName = "SecWeaponSocket";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera",
													meta = (ClampMin = 10.0,ClampMax = 40.0))
		float ZoomInterpSpeed = 20.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float ZoomedFOV = 65.0f;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void AuthSetPlayerColor(const FLinearColor& NewColor);
protected:
	void Throw();
	void ClearBeam();
	void AddNewBeam(FVector const NewPoint1,FVector const NewPoint2);
	void GetSegmentAtTime(FVector StartLocation, FVector InitialVelocity, FVector Gravity,
								float Time1, float Time2, FVector &OutPoint1, FVector &OutPoint2);
	void DrawingTrajectory();

private:
	UFUNCTION()
	void OnRep_PlayerColor() const;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	virtual FVector GetPawnViewLocation() const override;

	// Esto esta expuesto para poder usarse en el BT.
	/**
	UFUNCTION(BlueprintCallable,Category = "Player")
	void  StartFire();
	UFUNCTION(BlueprintCallable,Category = "Player")
	void StopFire();*/

	// Input interface
	virtual void I_Jump() override;
	virtual void I_Reload() override;
	virtual void I_ChangeWeapon() override;
	virtual void I_StartCrouch() override;
	virtual void I_StopCrouch() override;
	UFUNCTION(BlueprintCallable,Category = "Player")
	virtual void I_StartFire() override;
	UFUNCTION(BlueprintCallable,Category = "Player")
	virtual void I_StopFire() override;
	virtual void I_StartRun() override;
	virtual void I_StopRun() override;
	virtual void I_StartThrow() override;
	virtual void I_StopThrow() override;
	virtual void I_MoveForward(float Value) override;
	virtual void I_MoveRight(float Value) override;
	virtual void I_TurnRate(float Value) override;
	virtual void I_LookUpRate(float Value) override;
	virtual void I_StartADS() override;
	virtual void I_StopADS() override;

	virtual USHealthComponent* I_GetHealthComp() const override;
};
