// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/SCharacter.h"

#include "CoopPlayerController.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "Weapons/SWeapon.h"
#include "Weapons/SProjectile.h"
#include "Components/CapsuleComponent.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SHealthComponent.h"
#include "CoopTPS.h"
#include "Core/TPSHud.h"
#include "Net/UnrealNetwork.h"
#include "UI/RoleMessage.h"

// console debugging
static bool DebugCharacter = false;
FAutoConsoleVariableRef CVarDebugGame(TEXT("Coop.ShowRole"), DebugCharacter,
											TEXT("Show Pawn Role"), ECVF_Cheat);
DECLARE_STATS_GROUP(TEXT("CoopGame"), STATGROUP_PlayerChar, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("CoopGame"),STAT_RayForDoors,STATGROUP_PlayerChar);
DECLARE_CYCLE_STAT(TEXT("CoopGame"),STAT_RayForItems,STATGROUP_PlayerChar);
// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance
 	// if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// spring arm
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);


	// de esta manera la capsula no bloqueara nuestra arma.
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Throwing granade Arc
	BeamComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(SpringArmComp);
	BeamComp->bAutoActivate = false;

	BeamEndPointDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BeamEndPointDecal"));
	BeamEndPointDecal->SetVisibility(false);

	InfoWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidgetcomp"));
	InfoWidgetComp->SetupAttachment(RootComponent);
	
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (PawnMesh)
	{
		MeshComponent->SetSkeletalMesh(PawnMesh);
		MeshComponent->SetCollisionObjectType(COLLISION_PAWN);
		MeshComponent->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
		MeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
		MeshComponent->SetCollisionResponseToChannel(COLLISION_WEAPON,ECR_Block);
		MeshID = MeshComponent->CreateDynamicMaterialInstance(0);
		// this line is for crouch
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
		//bUseControllerRotationYaw=true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed; 
	}
}

// Esta funcion es llamada desde el Gamemode para señalar el cambio de color segun el player controller. 
void ASCharacter::AuthSetPlayerColor(const FLinearColor& NewColor)
{
	checkf(HasAuthority(), TEXT("ASCharacter::AuthSetPlayerColor called on Client"));
	PlayerColor = NewColor;
	OnRep_PlayerColor();
}

const FHitResult& ASCharacter::GetHitTrace() const
{
	return TraceResult;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	if(bShowRole)
	{
		if(const auto WidgetRole =Cast<URoleMessage>(InfoWidgetComp->GetUserWidgetObject()))
		{
			WidgetRole->ShowPlayerNetRole(this);
		}
	}

	DefaultFOV = CameraComp->FieldOfView;
	// health start
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector,
											FRotator::ZeroRotator, SpawnParams);
		SecondaryWeapon = GetWorld()->SpawnActor<ASWeapon>(SecondaryWeaponClass,
						FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
		if(SecondaryWeapon)
		{
			SecondaryWeapon->SetOwner(this);
			SecondaryWeapon->AttachToComponent(GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale, SecondaryWeaponSocketName);
		}
	}
}

void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateAimOffset();
	// in grenade mode
	if (bIsGranadaMode)
	{
		LaunchDistance  = FMath::Clamp( LaunchDistance + 10.0f, 1.0f, 1500.0f);
		InitialLocalVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance/2.0f);
		DrawingTrajectory();
	}
	SetHUDCrosshairs(DeltaTime);
	// is true : first - false : second
	float const TargetFOV = bIsAiming ? ZoomedFOV : DefaultFOV;
	float const NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime,
																				ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

void ASCharacter::I_MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}
void ASCharacter::I_MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::I_TurnRate(float Value)
{
	AddControllerYawInput(Value);
}

void ASCharacter::I_LookUpRate(float Value)
{
	AddControllerPitchInput(Value);
}

void ASCharacter::I_StartCrouch()
{
	Crouch();
}

void ASCharacter::I_StopCrouch()
{
	UnCrouch();
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ASCharacter::I_Jump()
{
	if(CanJump())
	{
		Jump();
	}
}

void ASCharacter::I_StartAiming()
{
	bIsAiming = true;
	GetCharacterMovement()->MaxWalkSpeed=AimWalkSpeed; 
	ServerAiming(true);
}

void ASCharacter::I_StopAiming()
{
	bIsAiming = false;
	GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed; 
	ServerAiming(false);
}

USHealthComponent* ASCharacter::I_GetHealthComp() const
{
	return HealthComp;
}

bool ASCharacter::IsWeaponEquipped() const
{
	return CurrentWeapon != nullptr;
}

bool ASCharacter::IsAiming() const
{
	return bIsAiming;
}

FTransform ASCharacter::GetHandlingWeaponTransform() const
{
	if(CurrentWeapon)
	{
		return CurrentWeapon->GetWeaponHandle();
	}
	return FTransform{};
}

void ASCharacter::I_StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
		PlayMontage(CurrentWeapon->GetFireMontage());
	}
}

void ASCharacter::I_StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2);
	}
}

void ASCharacter::I_StartRun()
{
}

void ASCharacter::I_StopRun()
{
}

void ASCharacter::I_Reload()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StartReloading();
		PlayMontage(CurrentWeapon->GetReloadMontage());
	}
}

// TODO : make a Throwing Actor Component
void ASCharacter::Throw()
{
	if (GranadaClass)
	{
		ClearBeam();
		const auto& Grenade = GetWorld()->SpawnActorDeferred<ASProjectile>(GranadaClass,
											FTransform(SpawnRotation, StartLocation));
		
		if (Grenade)
		{
		Grenade->InitialLocalVelocity = InitialLocalVelocity;
		UGameplayStatics::FinishSpawningActor(Grenade, FTransform(SpawnRotation,
														StartLocation, SpawnScale));
		}		
	}
}

void ASCharacter::I_StartThrow()
{
	bIsGranadaMode = true;		
}

void ASCharacter::I_StopThrow()
{
	bIsGranadaMode = false;
	LaunchDistance = 100.0f;
	BeamEndPointDecal->SetVisibility(false);
	ClearBeam();
	Throw();
}

void ASCharacter::ClearBeam()
{
for(auto Beam = BeamArray.CreateIterator(); Beam; ++Beam)
{
	(*Beam)->DestroyComponent();
}
	BeamArray.Empty();
}

void ASCharacter::AddNewBeam(FVector const NewPoint1, FVector const NewPoint2)
{
	BeamComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamFX, NewPoint1,
											FRotator::ZeroRotator, true);
	BeamArray.Add(BeamComp);
	BeamComp->SetBeamSourcePoint(0, NewPoint1, 0);
	BeamComp->SetBeamTargetPoint(0, NewPoint2, 0);
}

void ASCharacter::GetSegmentAtTime(const FVector LocalStartLocation, const FVector LocalInitialVelocity,
									const FVector LocalGravity, const float LocalTime1, const float LocalTime2,
																FVector &OutPoint1, FVector &OutPoint2)
{
	OutPoint1 = (LocalStartLocation + (LocalInitialVelocity*LocalTime1) +
												(LocalGravity*(LocalTime1*LocalTime1*0.5f)));
	OutPoint2 = (LocalStartLocation + (LocalInitialVelocity*LocalTime2) +
												(LocalGravity*(LocalTime2*LocalTime2*0.5f)));
}

void ASCharacter::DrawingTrajectory()
{
	if (GranadaClass)
	{
		ClearBeam();
		constexpr float PathLifeTime = 5.0f;
		SpawnRotation = GetControlRotation();
		FVector GrenadeOffset = FVector(100.0f, 0.0f, -10.0f);
		FVector ThrowRotateVector = GetControlRotation().RotateVector(GrenadeOffset);
		StartLocation = GetMesh()->GetSocketLocation(GrenadeSocketName) + ThrowRotateVector;
		const FTransform TotalPosition(SpawnRotation, ThrowRotateVector, SpawnScale);

		FVector InitialVelocity = UKismetMathLibrary::TransformDirection(TotalPosition, InitialLocalVelocity);
		uint8 LastIndex = floor(PathLifeTime / TimeInterval); 
		for (uint8 i = 0; i <= LastIndex; i++)
		{
			float Time1 = i * TimeInterval;
			float Time2 = (i + 1) * TimeInterval;

			const FName TraceTag("TraceTag");
			FCollisionQueryParams QueryParams;
			QueryParams.TraceTag = TraceTag;
			FHitResult Hit;
			GetSegmentAtTime(StartLocation, InitialVelocity, Gravity, Time1, Time2,
															Point1,Point2);
			if (GetWorld()->LineTraceSingleByChannel(Hit, Point1, Point2,
															ECC_Visibility, QueryParams))
			{
				BeamEndPointDecal->SetVisibility(true);
				BeamEndPointDecal->SetWorldLocation(Hit.ImpactPoint);
				BeamEndPointDecal->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal));
								
				break;
			}
			AddNewBeam(Point1, Point2);
			BeamEndPointDecal->SetVisibility(false);
		}
	}
}

FLinearColor ASCharacter::IterationTrace()
{
	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation,EyeRotation);
	const FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 2000);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(this);
	if (FHitResult HitResult;GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd,
	                                                               COLLISION_WEAPON, QueryParams))
	{
		if(CurrentWeapon)
		{
			CurrentWeapon->SetHitResult(HitResult);
		}		
		if(DebugCharacter)
		{
			DrawDebugSphere(GetWorld(),HitResult.ImpactPoint,5.f,12,FColor::White,false,1.f,
			0,3);
		}
		return(USHealthComponent::IsFriendly(this, HitResult.GetActor())) ?
			FLinearColor::Green : FLinearColor::Red;
	}
	if(CurrentWeapon)
	{
		FHitResult HitResult;
		CurrentWeapon->SetHitResult(HitResult);
	}	
	return FLinearColor::White;
}

void ASCharacter::CalculateAimOffset()
{
	if(CurrentWeapon == nullptr){return;}
	if(GetVelocity().Size() == 0.f && !GetCharacterMovement()->IsFalling())
	{
		bUseControllerRotationYaw = false;
		const FRotator CurrentAimRotation =  FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		const FRotator DeltaAimRot = UKismetMathLibrary::NormalizedDeltaRotator(
												CurrentAimRotation,StartingAimRotation);
		AimOffsetYaw = DeltaAimRot.Yaw;
	}
	else
	{
		bUseControllerRotationYaw = true;
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AimOffsetYaw = 0.f;		
	}
	AimOffSetPitch = GetBaseAimRotation().Pitch;
	if(AimOffSetPitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270,360) to [-90,0)
		const FVector2D InRange(270.f,360.f);
		const FVector2D OutRange(-90.f,0.f);
		AimOffSetPitch = FMath::GetMappedRangeValueClamped(InRange,OutRange,
																					AimOffSetPitch);
	}
}

void ASCharacter::PlayMontage(UAnimMontage* MontageToPlay) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(IsValid(AnimInstance) )
	{
		AnimInstance->Montage_Play(MontageToPlay);
		const FName SectionName = bIsAiming? FName("Aiming") : FName("Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ASCharacter::I_ChangeWeapon()
{
	if (CurrentWeapon != SecondaryWeapon && HasAuthority())
	{
		ASWeapon* TempWeapon = SecondaryWeapon;
		SecondaryWeapon = CurrentWeapon;
		SecondaryWeapon->AttachToComponent(GetMesh(),
		                                   FAttachmentTransformRules::SnapToTargetIncludingScale,
		                                   SecondaryWeaponSocketName);
		CurrentWeapon = TempWeapon;
		CurrentWeapon->AttachToComponent(GetMesh(),
		                                 FAttachmentTransformRules::SnapToTargetIncludingScale,
		                                 WeaponSocketName);
	}
	else
	{
		ServerChangeWeapon();
	}
}
/*
void ASCharacter::OnRep_Aiming()
{
	GetCharacterMovement()->MaxWalkSpeed=bIsAiming? 300.f:600.f; 
}
*/
// Cambio del color del pawn segun el playercontroller que lo controle.
void ASCharacter::OnRep_PlayerColor() const
{
	if(MeshID)
	{
		MeshID->SetVectorParameterValue(FName("BodyColor"), PlayerColor);
	}
}

void ASCharacter::OnRep_WeaponEquipped()
{
	if(CurrentWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		//bUseControllerRotationYaw = true;
	}
}

void ASCharacter::ServerChangeWeapon_Implementation()
{
	ASWeapon* TempWeapon = SecondaryWeapon;
	SecondaryWeapon = CurrentWeapon;
	SecondaryWeapon->AttachToComponent(GetMesh(),
									   FAttachmentTransformRules::SnapToTargetIncludingScale,
									   SecondaryWeaponSocketName);
	CurrentWeapon = TempWeapon;
	CurrentWeapon->AttachToComponent(GetMesh(),
									 FAttachmentTransformRules::SnapToTargetIncludingScale,
									 WeaponSocketName);
}

void ASCharacter::SetHUDCrosshairs(float DeltaTime)
{
	if(this->Controller == nullptr) return;
	PlayerController = PlayerController == nullptr? Cast<ACoopPlayerController>(GetController())
																	: PlayerController;
	if(PlayerController)
	{
		HUD = HUD == nullptr? Cast<ATPSHud>(PlayerController->GetHUD()) : HUD;
		if(HUD)
		{
			FHUDData HUDData;
			if(CurrentWeapon)
			{
				HUDData.CrosshairsCenter = CurrentWeapon->GetCrosshairData().CrosshairsCenter;
				HUDData.CrosshairsLeft = CurrentWeapon->GetCrosshairData().CrosshairsLeft;
				HUDData.CrosshairsRight = CurrentWeapon->GetCrosshairData().CrosshairsRight;
				HUDData.CrosshairsUp = CurrentWeapon->GetCrosshairData().CrosshairsUp;
				HUDData.CrosshairsDown = CurrentWeapon->GetCrosshairData().CrosshairsDown;
			}
			else
			{
				HUDData.CrosshairsCenter = nullptr;
				HUDData.CrosshairsLeft = nullptr;
				HUDData.CrosshairsRight = nullptr;
				HUDData.CrosshairsUp = nullptr;
				HUDData.CrosshairsDown = nullptr;
			}
			const FVector2d WalkSpeedRange(0.0f, GetCharacterMovement()->MaxWalkSpeed);
			const FVector2d VelocityMultiplierRange(0.0f,1.f);
			FVector Velocity = GetVelocity();
			Velocity.Z = 0.f;
			CrosshairSpreadFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,
									VelocityMultiplierRange,Velocity.Size());
			if(GetCharacterMovement()->IsFalling())
			{
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,2.25f,DeltaTime,
																				2.25f);
			}
			else if (CurrentWeapon)
			{
				const float ReloadingFactor = CurrentWeapon->IsReloading() ? 5.f : 1.f;
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,ReloadingFactor,
																	DeltaTime,2.25f);
			}
			else
			{
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,0.f,DeltaTime,
																				30.f);
			}
			
			HUDData.CrosshairSpread = CrosshairSpreadFactor + CrossInAirFactor;
			HUDData.CrosshairColor = IterationTrace();
			HUD->SetHUDData(HUDData);
		}
	}
}

void ASCharacter::ServerAiming_Implementation(const bool bAiming)
{
	bIsAiming = bAiming;
	GetCharacterMovement()->MaxWalkSpeed=bIsAiming? AimWalkSpeed:BaseWalkSpeed; 
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, const float Health,
                                  float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy,
                                  AActor* DamageCauser)
{
	if (Health<=0.0f && !bDied)
	{
		bDied = true;
		if(CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(10);
	}
}
// networking --> 
void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, PrimaryWeapon);
	DOREPLIFETIME(ASCharacter, SecondaryWeapon);
	DOREPLIFETIME(ASCharacter, PlayerColor);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, bIsAiming);
}