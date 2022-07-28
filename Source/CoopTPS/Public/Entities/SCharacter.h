// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Interfaces/IHealthyActor.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInputComm.h"
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
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
		class USHealthComponent* HealthComp;
	UPROPERTY(VisibleAnywhere)
		class URagDollStateComp* RagDollComp;
	UPROPERTY(VisibleAnywhere)
		class ULagCompensationComp* LagCompensationComp;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Health")
		bool bDied = false;
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
		bool bIsAiming=false;
	UFUNCTION()
		void OnRep_Aiming();
	bool bAimButtonPressed=false;
	UPROPERTY(EditAnywhere, Category = "Settings")
		USkeletalMesh* PawnMesh = nullptr;	
	// aiming variables
	bool bTurnInPlace=false;
	float InterpAimYaw=0.f;
	float AimOffsetYaw=0.f;
	float AimOffSetPitch=0.f;
	FRotator StartingAimRotation=FRotator::ZeroRotator;
	float DefaultFOV;

	// grenade mode
	bool bIsGranadaMode = false;
	float LaunchDistance = 100.0f;
	FVector InitialLocalVelocity;
	FVector StartLocation;
	FVector SpawnScale = FVector(1.0f);
	FRotator SpawnRotation;
	FVector Point1;
	FVector Point2;
	UPROPERTY()
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

public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponChanged, Transient)
		ASWeapon* CurrentWeapon= nullptr;
	UPROPERTY(Replicated,Transient)
		ASWeapon* OverlappingWeapon = nullptr;
	UPROPERTY(Replicated, Transient) //replicated ReplicatedUsing = OnRep_SecondaryWeaponChanged
		ASWeapon* SecondaryWeapon = nullptr;
		
	UPROPERTY(ReplicatedUsing=OnRep_PlayerColor,EditAnywhere,BlueprintReadOnly,Transient)
	FLinearColor PlayerColor;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Animations")
	UAnimMontage* HitReactMontage=nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement")
		float BaseWalkSpeed=400.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement")
		float AimWalkSpeed=200.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Weapon")
		TSubclassOf<ASWeapon>StarterWeaponClass;
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Weapon")
		TSubclassOf<ASWeapon>SecondaryWeaponClass;

		const FName WeaponSocketName = "WeaponSocket";
		const FName SecondaryWeaponSocketName = "SecWeaponSocket";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Camera",
													meta = (ClampMin = 10.0,ClampMax = 40.0))
		float ZoomInterpSpeed = 20.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Camera")
		float ZoomedFOV = 65.0f;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void SetHUDCrosshairs(float DeltaTime);
public:
	//Server-Side
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
	FString ShowPlayerNetRole() const;
	void PickupWeapon();
	void SwapWeapons();
	void TurnInPlace();
	void StartAiming();
	void StopAiming();
	void CalculateAimOffset();
	UFUNCTION(NetMulticast,Unreliable)
	void Multicast_PlayMontage(UAnimMontage* MontageToPlay) const;
	FHitResult TraceResult;
	float CrosshairSpreadFactor;
	float CrossInAirFactor;
	UPROPERTY()
	class ATPSHud* HUD =nullptr;
	UPROPERTY()
	class ACoopPlayerController* PlayerController=nullptr;
	UFUNCTION()
	void OnRep_PlayerColor() const;
	UFUNCTION()
	void OnRep_CurrentWeaponChanged() const;
	//UFUNCTION()
	//void OnRep_SecondaryWeaponChanged();
	
	UFUNCTION(Server,Reliable)
	void ServerSwapWeapon();
	UFUNCTION(Server,Reliable)
	void ServerReload();
	UFUNCTION(Server,Reliable)
	void ServerAiming(bool bAiming);
	UFUNCTION(Server,Reliable)
	void ServerPickupWeapon();
	UFUNCTION()
	void HealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
		const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
protected:
	//Lag comp
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_Head;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_Pelvis;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_Spine_02;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_Spine_03;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_UpperArm_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_UpperArm_r;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_lowerarm_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_lowerarm_r;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_hand_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_hand_r;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_thigh_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_thigh_r;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_calf_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_calf_r;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_foot_l;
	UPROPERTY(EditAnywhere,Category="RewindSystem")
	UBoxComponent* HitBox_foot_r;

	UPROPERTY()
	TMap<FName,UBoxComponent*> ServerRewindHitBoxes;
	void CreateRewindHitBox();
public:
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual FVector GetPawnViewLocation() const override;
	UFUNCTION()
	const FHitResult& GetHitTrace()const;
	// Input interface
	virtual void I_Jump() override;
	virtual void I_Reload() override;
	virtual void I_SwapWeapons() override;
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
	virtual void I_PickupWeapon() override;
	virtual void I_DropWeapon() override;
	virtual void I_SetOverlappingWeapon(ASWeapon* NewWeapon) override;
	virtual USHealthComponent* I_GetHealthComp() const override;
	
	bool IsWeaponEquipped() const;
	const TMap<FName,UBoxComponent*>& GetRewindHitBoxes() const;
	UFUNCTION(BlueprintCallable)
	bool IsAiming()const;
	UFUNCTION()
	FTransform GetHandlingWeaponTransform() const;
	UFUNCTION()
	FVector2D GetAimOffsetData()const{return FVector2D(AimOffsetYaw,AimOffSetPitch);}
	FORCEINLINE bool PlayerIsDead()const{return bDied;}
	FORCEINLINE ULagCompensationComp* GetLagCompensationComp() const{return LagCompensationComp;}
};
