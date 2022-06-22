// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSHud.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInputComm.h"
#include "Components/WidgetComponent.h"
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
	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere)
		class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|GrenadeMode")
		UParticleSystem* BeamFX;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|GrenadeMode")
		UParticleSystemComponent* BeamComp;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|GrenadeMode")
		UDecalComponent* BeamEndPointDecal;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Settings|HealthComp")
		class USHealthComponent* HealthComp;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
			UWidgetComponent* InfoWidgetComp;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Health")
		bool bDied = false;
	UPROPERTY(Replicated)
		bool bIsAiming;
	UPROPERTY(EditAnywhere, Category = "Settings")
		USkeletalMesh* PawnMesh = nullptr;

	// aiming variables
	
	float DefaultFOV;

	// grenade mode
	//UPROPERTY()
	//class ASProjectile* Grenade = nullptr;
	bool bIsGranadaMode = false;
	float LaunchDistance = 100.0f;
	//FVector InitialVelocity;
	FVector InitialLocalVelocity;
	FVector StartLocation;
	FVector SpawnScale = FVector(1.0f);
	FRotator SpawnRotation;
	FVector Point1;
	FVector Point2;
	TArray<UParticleSystemComponent*>BeamArray;
	const float TimeInterval = 0.05;
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
	// TODO : ver de quitar el tema de primario y current. Current y secondary deberia quedar.
	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquipped, Transient)
		class ASWeapon* CurrentWeapon= nullptr;
	UPROPERTY(Replicated, Transient)
		ASWeapon* PrimaryWeapon = nullptr;
	UPROPERTY(Replicated, Transient)
		ASWeapon* SecondaryWeapon = nullptr;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerColor,EditAnywhere,BlueprintReadOnly,Transient)
	FLinearColor PlayerColor;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement")
		float BaseWalkSpeed=400.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement")
		float AimWalkSpeed=200.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Weapon")
		TSubclassOf<ASWeapon>StarterWeaponClass;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Weapon")
		TSubclassOf<ASWeapon>SecondaryWeaponClass;
	UPROPERTY(VisibleDefaultsOnly, Category = "Settings|Weapon")
		FName WeaponSocketName = "WeaponSocket";
	UPROPERTY(VisibleDefaultsOnly, Category = "Settings|Weapon")
		FName SecondaryWeaponSocketName = "SecWeaponSocket";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Camera",
													meta = (ClampMin = 10.0,ClampMax = 40.0))
		float ZoomInterpSpeed = 20.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Camera")
		float ZoomedFOV = 65.0f;

	UPROPERTY(EditAnywhere)
		bool bShowRole=true;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SetHUDCrosshairs(float DeltaTime);
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
	FLinearColor IterationTrace();
private:
	void PlayMontage(UAnimMontage* MontageToPlay) const;
	FHitResult TraceResult;
	float CrosshairSpreadFactor;
	float CrossInAirFactor;
	UPROPERTY()
	ATPSHud* HUD =nullptr;
	UPROPERTY()
	class ACoopPlayerController* PlayerController=nullptr;
	//UFUNCTION()
	//void OnRep_Aiming();
	UFUNCTION()
	void OnRep_PlayerColor() const;
	UFUNCTION()
	void OnRep_WeaponEquipped();
	UFUNCTION(Server,Reliable)
	void ServerChangeWeapon();
	UFUNCTION(Server,Reliable)
	void ServerAiming(bool bAiming);
	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
		const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	virtual FVector GetPawnViewLocation() const override;
	UFUNCTION()
	const FHitResult& GetHitTrace()const;
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
	virtual void I_StartAiming() override;
	virtual void I_StopAiming() override;
	
	virtual USHealthComponent* I_GetHealthComp() const override;

public:
	bool IsWeaponEquipped() const;
	UFUNCTION(BlueprintCallable)
	bool IsAiming()const;
	
};
