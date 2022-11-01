// Fill out your copyright notice in the Description page of Project Settings.

#include "Entities/SCharacter.h"

#include "Core/CoopPlayerController.h"
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
#include "Components/LagCompensationComp.h"
#include "CoopTPS.h"
#include "SPlayerState.h"
#include "Components/RagDollStateComp.h"
#include "Core/TPSHud.h"
#include "Net/UnrealNetwork.h"
#include "UI/RoleMessage.h"

// console debugging
static bool DebugCharacter = false;
FAutoConsoleVariableRef CVarDebugPawn(TEXT("Coop.DebugPawn"), DebugCharacter,
											TEXT("Show Pawn Debug"), ECVF_Cheat);
DECLARE_STATS_GROUP(TEXT("CoopGame"), STATGROUP_PlayerChar, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("CoopGame"),STAT_RayForDoors,STATGROUP_PlayerChar);
DECLARE_CYCLE_STAT(TEXT("CoopGame"),STAT_RayForItems,STATGROUP_PlayerChar);

ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw=false;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Throwing granade Arc
	BeamComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(SpringArmComp);
	BeamComp->bAutoActivate = false;

	BeamEndPointDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BeamEndPointDecal"));
	BeamEndPointDecal->SetVisibility(false);
	if(GetMesh())
	{
		CreateRewindHitBox();
	}
	
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->TeamNum = 0;
	
	RagDollComp = CreateDefaultSubobject<URagDollStateComp>(TEXT("RagDollComp"));
	
	LagCompensationComp = CreateDefaultSubobject<ULagCompensationComp>(TEXT("LagCompensationComp"));
	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	CurrentWeaponTag = NoWeaponTag;
}

UAbilitySystemComponent* ASCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if(const ASPlayerState* LocalPlayerState = GetPlayerState<ASPlayerState>())
	{
		AbilitySystemComponent = LocalPlayerState->GetAbilitySystemComponent();
		AttributeSetBase = LocalPlayerState->GetAttributeSet();
	}
}

void ASCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent,
			FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"),
			FString("EGSAbilityInputID"),
			static_cast<int32>(EGSAbilityInputID::Confirm),
			static_cast<int32>(EGSAbilityInputID::Cancel)));

		bASCInputBound = true;
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ASCharacter::TryFire()
{
}

void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BindASCInput();
}

void ASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if(ASPlayerState* PS = GetPlayerState<ASPlayerState>())
	{
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(PS,this);
		BindASCInput();
		AttributeSetBase = PS->GetAttributeSet();
		AbilitySystemComponent->SetTagMapCount(DeadTag,0);
	}
	
}

// networking --> 
void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, SecondaryWeapon);
	DOREPLIFETIME(ASCharacter, OverlappingWeapon);
	DOREPLIFETIME(ASCharacter, PlayerColor);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, bIsAiming);
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
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed; 
	}
}

// Esta funcion es llamada desde el Gamemode para se?alar el cambio de color segun el player controller. 
void ASCharacter::AuthSetPlayerColor(const FLinearColor& NewColor)
{
	PlayerColor = NewColor;
	OnRep_PlayerColor();
}

const FHitResult& ASCharacter::GetHitTrace() const
{
	return TraceResult;
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;
	if (HasAuthority())
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::HealthChanged);
		/*
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if(StarterWeaponClass)
		{
			CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector,
									FRotator::ZeroRotator, SpawnParams);
			if(IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetOwner(this);
				CurrentWeapon->EquipWeapon(GetMesh(), WeaponSocketName);
			}
		}
		if(SecondaryWeaponClass)
		{
			SecondaryWeapon = GetWorld()->SpawnActor<ASWeapon>(SecondaryWeaponClass,
				FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if(IsValid(SecondaryWeapon))
			{
				SecondaryWeapon->SetOwner(this);
				SecondaryWeapon->EquipWeapon(GetMesh(), SecondaryWeaponSocketName);
			}
		}
		//OnRep_WeaponEquipped();
	}
	if(IsValid(CurrentWeapon))
	{
		CurrentWeapon->SetInitialInfoUI();*/
	}
}

void ASCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(IsValid(CurrentWeapon)){CurrentWeapon->Destroy();}
	if(IsValid(SecondaryWeapon)){SecondaryWeapon->Destroy();}
	HealthComp->OnHealthChanged.RemoveDynamic(this,&ASCharacter::HealthChanged);
	if(AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}
	Super::EndPlay(EndPlayReason);
}

void ASCharacter::CreateRewindHitBox()
{
	HitBox_Head = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_Head"));
	HitBox_Head->SetupAttachment(GetMesh(),FName("head"));
	HitBox_Head->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("head"),HitBox_Head);
	HitBox_Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_Pelvis"));
	HitBox_Pelvis->SetupAttachment(GetMesh(),FName("pelvis"));
	HitBox_Pelvis->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("pelvis"),HitBox_Pelvis);
	HitBox_Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_Spine_02"));
	HitBox_Spine_02->SetupAttachment(GetMesh(),FName("spine_02"));
	HitBox_Spine_02->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("spine_02"),HitBox_Spine_02);
	HitBox_Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_Spine_03"));
	HitBox_Spine_03->SetupAttachment(GetMesh(),FName("spine_03"));
	HitBox_Spine_03->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("spine_03"),HitBox_Spine_03);
	HitBox_UpperArm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_UpperArm_l"));
	HitBox_UpperArm_l->SetupAttachment(GetMesh(),FName("upperarm_l"));
	HitBox_UpperArm_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("upperarm_l"),HitBox_UpperArm_l);
	HitBox_UpperArm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_UpperArm_r"));
	HitBox_UpperArm_r->SetupAttachment(GetMesh(),FName("upperarm_r"));
	HitBox_UpperArm_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("upperarm_r"),HitBox_UpperArm_r);
	HitBox_lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_lowerarm_l"));
	HitBox_lowerarm_l->SetupAttachment(GetMesh(),FName("lowerarm_l"));
	HitBox_lowerarm_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("lowerarm_l"),HitBox_lowerarm_l);
	HitBox_lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_lowerarm_r"));
	HitBox_lowerarm_r->SetupAttachment(GetMesh(),FName("lowerarm_r"));
	HitBox_lowerarm_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("lowerarm_r"),HitBox_lowerarm_r);
	HitBox_hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_hand_l"));
	HitBox_hand_l->SetupAttachment(GetMesh(),FName("hand_l"));
	HitBox_hand_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("hand_l"),HitBox_hand_l);
	HitBox_hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_hand_r"));
	HitBox_hand_r->SetupAttachment(GetMesh(),FName("hand_r"));
	HitBox_hand_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("hand_r"),HitBox_hand_r);
	HitBox_thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_thigh_l"));
	HitBox_thigh_l->SetupAttachment(GetMesh(),FName("thigh_l"));
	HitBox_thigh_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("thigh_l"),HitBox_thigh_l);
	HitBox_thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_thigh_r"));
	HitBox_thigh_r->SetupAttachment(GetMesh(),FName("thigh_r"));
	HitBox_thigh_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("thigh_r"),HitBox_thigh_r);
	HitBox_calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_calf_l"));
	HitBox_calf_l->SetupAttachment(GetMesh(),FName("calf_l"));
	HitBox_calf_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("calf_l"),HitBox_calf_l);
	HitBox_calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_calf_r"));
	HitBox_calf_r->SetupAttachment(GetMesh(),FName("calf_r"));
	HitBox_calf_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("calf_r"),HitBox_calf_r);
	HitBox_foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_foot_l"));
	HitBox_foot_l->SetupAttachment(GetMesh(),FName("foot_l"));
	HitBox_foot_l->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("foot_l"),HitBox_foot_l);
	HitBox_foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox_foot_r"));
	HitBox_foot_r->SetupAttachment(GetMesh(),FName("foot_r"));
	HitBox_foot_r->SetComponentTickEnabled(false);
	ServerRewindHitBoxes.Add(FName("foot_r"),HitBox_foot_r);
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
	float const NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime,ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

void ASCharacter::I_MoveForward(float Value)
{
	if(IsLocallyControlled())
	{
		AddMovementInput(GetActorForwardVector() * Value);
	}	
}
void ASCharacter::I_MoveRight(float Value)
{
	if(IsLocallyControlled())
	{
		AddMovementInput(GetActorRightVector() * Value);
	}
}

void ASCharacter::I_TurnRate(float Value)
{
	if(IsLocallyControlled())
	{
		AddControllerYawInput(Value);
	}
}

void ASCharacter::I_LookUpRate(float Value)
{
	if(IsLocallyControlled())
	{
		AddControllerPitchInput(Value);
	}
}

void ASCharacter::I_StartCrouch()
{
	if(IsLocallyControlled())
	{
		Crouch();
	}
}

void ASCharacter::I_StopCrouch()
{
	if(IsLocallyControlled())
	{
		UnCrouch();
	}
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (IsValid(CameraComp))
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ASCharacter::I_Jump()
{
	if(IsLocallyControlled())
	{
		if(CanJump())
		{
			Jump();
		}
	}
}

void ASCharacter::OnRep_Aiming()
{
	if(IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
	}
}

void ASCharacter::I_StartAiming()
{
	if(IsLocallyControlled())
	{
		StartAiming();
		bAimButtonPressed = bIsAiming;
	}
}

void ASCharacter::I_StopAiming()
{
	if(IsLocallyControlled())
	{
		StopAiming();
		bAimButtonPressed = bIsAiming;
	}
}

void ASCharacter::I_PickupWeapon()
{
	if(IsValid(CurrentWeapon) && CurrentWeapon->IsReloading()){return;}
	
	if(HasAuthority())
	{
		PickupWeapon();
	}
	else
	{
		ServerPickupWeapon();
	}
}

void ASCharacter::I_DropWeapon()
{
	if(IsValid(CurrentWeapon))
	{
		CurrentWeapon->DropWeapon();
		CurrentWeapon = nullptr;
	}
	if(IsValid(SecondaryWeapon))
	{
		CurrentWeapon = std::move(SecondaryWeapon);
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->EquipWeapon(this,GetMesh(), WeaponSocketName);
		SecondaryWeapon = nullptr;
	}
}

void ASCharacter::I_SetOverlappingWeapon(ASWeapon* NewWeapon)
{
	OverlappingWeapon = std::move(NewWeapon);
}

USHealthComponent* ASCharacter::I_GetHealthComp() const
{
	return HealthComp;
}

bool ASCharacter::IsWeaponEquipped() const
{
	return CurrentWeapon != nullptr;
}

const TMap<FName, UBoxComponent*>& ASCharacter::GetRewindHitBoxes() const
{
	return ServerRewindHitBoxes;
}

bool ASCharacter::IsAiming() const
{
	return bIsAiming;
}

FTransform ASCharacter::GetHandlingWeaponTransform() const
{
	if(IsValid(CurrentWeapon))
	{
		return CurrentWeapon->GetWeaponHandle();
	}
	return FTransform{};
}

void ASCharacter::I_StartFire()
{
	if (IsValid(CurrentWeapon))
	{
		if(CurrentWeapon->IsReloading()) 
		{
			return;
		}
		CurrentWeapon->StartFire();
		Multicast_PlayMontage(CurrentWeapon->GetFireMontage());
	}
}

void ASCharacter::I_StopFire()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->StopFire();
		if(GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentWeapon->GetFireMontage()))
		{
			GetMesh()->GetAnimInstance()->Montage_Stop(0.2);
		}	
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
	if(IsValid(CurrentWeapon))
	{
		if(CurrentWeapon->GetCurrentAmmo() == CurrentWeapon->GetWeaponMaxAmmo() ||
			CurrentWeapon->IsReloading() || 
			!CurrentWeapon->HaveAmmoInMag())
		{
			//TODO: drop a sound or hint that it cant reload
			UE_LOG(LogTemp,Warning,TEXT("CANT RELOAD!"));
			return;
		}
		ServerReload();
	}
}

void ASCharacter::I_SwapWeapons()
{
	if(!IsValid(SecondaryWeapon)){return;}
	if(IsValid(CurrentWeapon) && CurrentWeapon->IsReloading()){return;}
	if (HasAuthority())
	{
		SwapWeapons();		
	}
	else
	{
		ServerSwapWeapon();
	}	
}

// TODO : make a Throwing Actor Component
void ASCharacter::Throw()
{
	if (GranadaClass)
	{
		ClearBeam();

		if (const auto& Grenade = GetWorld()->SpawnActorDeferred<ASProjectile>(GranadaClass,
														FTransform(SpawnRotation, StartLocation)))
		{
		Grenade->InitialLocalVelocity = InitialLocalVelocity;
		UGameplayStatics::FinishSpawningActor(Grenade, FTransform(SpawnRotation,StartLocation, SpawnScale));
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

// ReSharper disable once CppMemberFunctionMayBeStatic
void ASCharacter::GetSegmentAtTime(const FVector LocalStartLocation, const FVector LocalInitialVelocity,
                                   const FVector LocalGravity, const float LocalTime1, const float LocalTime2,
                                   FVector &OutPoint1, FVector &OutPoint2)
{
	OutPoint1 = (LocalStartLocation + (LocalInitialVelocity*LocalTime1) + (LocalGravity*(LocalTime1*LocalTime1*0.5f)));
	OutPoint2 = (LocalStartLocation + (LocalInitialVelocity*LocalTime2) + (LocalGravity*(LocalTime2*LocalTime2*0.5f)));
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
			GetSegmentAtTime(StartLocation, InitialVelocity, Gravity, Time1, Time2,
																Point1,Point2);
			if (FHitResult Hit; GetWorld()->LineTraceSingleByChannel(Hit, Point1, Point2,
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
		if(DebugCharacter)
		{
			DrawDebugSphere(GetWorld(),HitResult.ImpactPoint,5.f,12,FColor::White,false,
																				1.f,0,3);
		}
		return(USHealthComponent::IsFriendly(this, HitResult.GetActor())) ?
			FLinearColor::Green : FLinearColor::Red;
	}
	return FLinearColor::White;
}

void ASCharacter::PickupWeapon()
{
	if(IsValid(OverlappingWeapon))
	{
		if (IsValid(CurrentWeapon))
		{
			if(IsValid(SecondaryWeapon))
			{
				SecondaryWeapon->DropWeapon();
			}
			SecondaryWeapon = std::move(CurrentWeapon);
			SecondaryWeapon->SetOwner(this);
			SecondaryWeapon->EquipWeapon(this,GetMesh(), SecondaryWeaponSocketName);
			CurrentWeapon = std::move(OverlappingWeapon);
			OverlappingWeapon =nullptr;
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->EquipWeapon(this,GetMesh(), WeaponSocketName);
		}
		else
		{
			CurrentWeapon = std::move(OverlappingWeapon);
			OverlappingWeapon =nullptr;
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->EquipWeapon(this,GetMesh(), WeaponSocketName);
		}
		if(IsLocallyControlled() && HasAuthority())
		{
			PlayerController->SetWeaponInfoHUD(CurrentWeapon->GetWeaponName(),CurrentWeapon->GetCurrentAmmo(),
			CurrentWeapon->GetAmmoInBackpack());
		}
	}
	else
	{
		if (IsValid(CurrentWeapon))
		{
			CurrentWeapon->DropWeapon();
			CurrentWeapon =nullptr;
	
			if(IsLocallyControlled() && HasAuthority())
			{
				PlayerController->SetWeaponInfoHUD();
			}
		}
	}
}

void ASCharacter::SwapWeapons()
{
	if(!SecondaryWeapon){return;}
	ASWeapon* TempWeapon = std::move(SecondaryWeapon);
	if(CurrentWeapon)
	{
		SecondaryWeapon = std::move(CurrentWeapon);
		SecondaryWeapon->EquipWeapon(this,GetMesh(), SecondaryWeaponSocketName);
	}
	else
	{
		SecondaryWeapon =nullptr;
	}
	CurrentWeapon = std::move(TempWeapon);
	CurrentWeapon->EquipWeapon(this,GetMesh(), WeaponSocketName);
	if(IsLocallyControlled())
	{
		PlayerController->SetWeaponInfoHUD(CurrentWeapon->GetWeaponName(),CurrentWeapon->GetCurrentAmmo(),
		CurrentWeapon->GetAmmoInBackpack());
	}	
}

void ASCharacter::TurnInPlace()
{
	if(AimOffsetYaw > 90.f)
	{
		bTurnInPlace=true;
	}
	else if (AimOffsetYaw < -90.f)
	{
		bTurnInPlace=true;
	}
	if(bTurnInPlace)
	{
		InterpAimYaw = FMath::FInterpTo(InterpAimYaw,0.f,GetWorld()->GetDeltaSeconds(),5.f);
		AimOffsetYaw = InterpAimYaw;
		if(FMath::Abs(AimOffsetYaw)<15.f)
		{
			bTurnInPlace = false;
			StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		}
	}
}

void ASCharacter::StartAiming()
{
	bIsAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	if(!HasAuthority())
	{
		ServerAiming(true);
	}
}

void ASCharacter::StopAiming()
{
	bIsAiming = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	if(!HasAuthority())
	{
		ServerAiming(false);	
	}
}

void ASCharacter::CalculateAimOffset()
{
	if(CurrentWeapon == nullptr)
	{
		bUseControllerRotationYaw=true;
		return;
	}
	if(GetVelocity().Size() == 0.f && !GetCharacterMovement()->IsFalling())
	{
		const FRotator CurrentAimRotation =  FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		const FRotator DeltaAimRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,
																				StartingAimRotation);
		AimOffsetYaw = DeltaAimRot.Yaw;
		if(!bTurnInPlace)
		{
			InterpAimYaw = AimOffsetYaw;
		}
		bUseControllerRotationYaw=true;
		TurnInPlace();
	}
	else
	{
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AimOffsetYaw = 0.f;
		bUseControllerRotationYaw=true;
		bTurnInPlace=false;
	}
	AimOffSetPitch = GetBaseAimRotation().Pitch;
	if(AimOffSetPitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270,360) to [-90,0)
		const FVector2D InRange(270.f,360.f);
		const FVector2D OutRange(-90.f,0.f);
		AimOffSetPitch = FMath::GetMappedRangeValueClamped(InRange,OutRange,AimOffSetPitch);
	}
}

void ASCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay) const
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	IsValid(AnimInstance) )
	{
		AnimInstance->Montage_Play(MontageToPlay);
		const FName SectionName = bIsAiming? FName("Aiming") : FName("Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// Cambio del color del pawn segun el playercontroller que lo controle.
void ASCharacter::OnRep_PlayerColor() const
{
	if(MeshID)
	{
		MeshID->SetVectorParameterValue(FName("BodyColor"), PlayerColor);
	}
}

void ASCharacter::OnRep_CurrentWeaponChanged() const
{
	if(IsLocallyControlled() && !HasAuthority())
	{
		if(IsValid(CurrentWeapon))
		{
			PlayerController->SetWeaponInfoHUD(CurrentWeapon->GetWeaponName(),CurrentWeapon->GetCurrentAmmo(),
				CurrentWeapon->GetAmmoInBackpack());
			UE_LOG(LogTemp,Warning,TEXT("Character : Current ammo on %s : %d"),*ShowPlayerNetRole(),CurrentWeapon->GetCurrentAmmo());
		}
		else
		{
			PlayerController->SetWeaponInfoHUD();
		}
	}
}

/*
void ASCharacter::OnRep_SecondaryWeaponChanged()
{
	if(SecondaryWeapon)
	{
		SecondaryWeapon->SetOwner(this);
		SecondaryWeapon->EquipWeapon(GetMesh(), SecondaryWeaponSocketName);
	}
}
*/
void ASCharacter::ServerReload_Implementation()
{
	CurrentWeapon->StartReload();
	Multicast_PlayMontage(CurrentWeapon->GetReloadMontage());
}

void ASCharacter::ServerSwapWeapon_Implementation()
{
	SwapWeapons();
}

void ASCharacter::ServerAiming_Implementation(const bool bAiming)
{
	bIsAiming = bAiming;
	GetCharacterMovement()->MaxWalkSpeed=bIsAiming? AimWalkSpeed:BaseWalkSpeed;
}

void ASCharacter::ServerPickupWeapon_Implementation()
{
	PickupWeapon();
}

void ASCharacter::HealthChanged(USHealthComponent* OwningHealthComp, const float Health,float HealthDelta,
                                const class UDamageType* DamageType, class AController* InstigatedBy,AActor* DamageCauser)
{
	Multicast_PlayMontage(HitReactMontage);

	if (Health<=0.0f && !bDied)
	{
		bDied = true;
		//RagDollComp->ToggleRagDoll(true);
		if(CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(4);
		//Backfromdead
		//RagDollComp->ToggleRagDoll(false);
	}
}

void ASCharacter::SetHUDCrosshairs(float DeltaTime)
{
	if(this->Controller == nullptr) return;
	PlayerController = PlayerController == nullptr? Cast<ACoopPlayerController>(GetController())	: PlayerController;
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
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,2.25f,DeltaTime,2.25f);
			}
			else if (CurrentWeapon)
			{
				const float ReloadingFactor = CurrentWeapon->IsReloading() ? 5.f : 1.f;
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,ReloadingFactor,DeltaTime,2.25f);
			}
			else
			{
				CrossInAirFactor = FMath::FInterpTo(CrossInAirFactor,0.f,DeltaTime,30.f);
			}
			
			HUDData.CrosshairSpread = CrosshairSpreadFactor + CrossInAirFactor;
			HUDData.CrosshairColor = IterationTrace();
			HUD->SetHUDData(HUDData);
		}
	}
}

FString ASCharacter::ShowPlayerNetRole() const
{
	const ENetRole LocalRole = this->GetLocalRole();
	FString LocRole;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		LocRole = FString("Authority");
		return LocRole;
	case ENetRole::ROLE_AutonomousProxy:
		LocRole = FString("AutonomousProxy");
		return LocRole;
	case ENetRole::ROLE_SimulatedProxy:
		LocRole = FString("SimulatedProxy");
		return LocRole;
	case ENetRole::ROLE_None:
		LocRole = FString("NONE");
		return LocRole;
	default:
		LocRole = FString("DEFAULT");
		return LocRole;
	}
}