// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "SWeapon.h"
#include "SProjectile.h"
#include "Components/CapsuleComponent.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SHealthComponent.h"
#include "CoopTPS.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// spring arm
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);


	// de esta manera la capsula no bloqueara nuestra arma.
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Throwing granade Arc
	BeamComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(SpringArmComp);
	BeamComp->bAutoActivate = false;

	BeamEndPointDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BeamEndPointDecal"));
	BeamEndPointDecal->SetVisibility(false);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// forward Backward
	PlayerInputComponent->BindAxis("MoveForward",this,&ASCharacter::MoveForward);
	// Right - Left
	PlayerInputComponent->BindAxis("MoveRight", this,&ASCharacter::MoveRight);
	
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::Reload);
	
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASCharacter::StartThrow);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &ASCharacter::StopThrow);

	PlayerInputComponent->BindAction("ChangeWeapon", IE_Released, this, &ASCharacter::SelectWeapon);
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (PawnMesh)
	{
		MeshComponent->SetSkeletalMesh(PawnMesh);
		MeshID = MeshComponent->CreateDynamicMaterialInstance(0);
		// this line is for crouch
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

// Esta funcion es llamada desde el Gamemode para señalar el cambio de color segun el player controller. 
void ASCharacter::AuthSetPlayerColor(const FLinearColor& NewColor)
{
	checkf(HasAuthority(), TEXT("ASCharacter::AuthSetPlayerColor called on Client"));
	PlayerColor = NewColor;
	OnRep_PlayerColor();
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	// health start
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	if (HasAuthority())
	{
		// spawn initial weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		SecondaryWeapon = GetWorld()->SpawnActor<ASWeapon>(SecondaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
		if(SecondaryWeapon)
		{
			SecondaryWeapon->SetOwner(this);
			SecondaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SecondaryWeaponSocketName);
		}
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// in grenade mode
	if (bIsGranadaMode)
	{
		LaunchDistance  = FMath::Clamp( LaunchDistance + 10.0f, 1.0f, 1500.0f);
		InitialLocalVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance/2.0f);
		DrawingTrajectory();
	}


	// is true : first - false : second
	float const TargetFOV = bIsZoomed ? ZoomedFOV : DefaultFOV;
	float const NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}
void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
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

void ASCharacter::BeginZoom()
{
	bIsZoomed = true;
}

void ASCharacter::EndZoom()
{
	bIsZoomed = false;
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::Reload()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StartReloading();
	}
}

// TODO : make a Throwing Actor Component
void ASCharacter::Throw()
{
	if (GranadaClass)
	{
		ClearBeam();
		const auto& Grenade = GetWorld()->SpawnActorDeferred<ASProjectile>(GranadaClass, FTransform(SpawnRotation, StartLocation));
		
		if (Grenade)
		{
		Grenade->InitialLocalVelocity = InitialLocalVelocity;
		UGameplayStatics::FinishSpawningActor(Grenade, FTransform(SpawnRotation, StartLocation, SpawnScale));
		}		
	}
}

void ASCharacter::StartThrow()
{
	bIsGranadaMode = true;		
}

void ASCharacter::StopThrow()
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
	
	BeamComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamFX, NewPoint1, FRotator::ZeroRotator, true);
	BeamArray.Add(BeamComp);

		BeamComp->SetBeamSourcePoint(0, NewPoint1, 0);
		BeamComp->SetBeamTargetPoint(0, NewPoint2, 0);
}

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

		SpawnRotation = GetControlRotation();
		FVector GrenadeOffset = FVector(100.0f, 0.0f, -10.0f);
		ThrowRotateVector = GetControlRotation().RotateVector(GrenadeOffset);
		StartLocation = GetMesh()->GetSocketLocation(GrenadeSocketName) + ThrowRotateVector;
		const FTransform TotalPosition(SpawnRotation, ThrowRotateVector, SpawnScale);

		InitialVelocity = UKismetMathLibrary::TransformDirection(TotalPosition, InitialLocalVelocity);
		uint8 LastIndex = floor(PathLifeTime / TimeInterval); 
		for (uint8 i = 0; i <= LastIndex; i++)
		{
			float Time1 = i * TimeInterval;
			float Time2 = (i + 1) * TimeInterval;

			// trace line
			const FName TraceTag("TraceTag");
			FCollisionQueryParams QueryParams;
			QueryParams.TraceTag = TraceTag;
			FHitResult Hit;
			GetSegmentAtTime(StartLocation, InitialVelocity, Gravity, Time1, Time2,Point1,Point2);
			if (GetWorld()->LineTraceSingleByChannel(Hit, Point1, Point2, ECC_Visibility, QueryParams))
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


void ASCharacter::SelectWeapon()
{
	if(CurrentWeapon != SecondaryWeapon)
	{
		if(HasAuthority())
		{
			ASWeapon* TempWeapon = SecondaryWeapon;
			SecondaryWeapon = CurrentWeapon;
			SecondaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SecondaryWeaponSocketName);
			CurrentWeapon = TempWeapon;
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocketName);
		}
	}
}

//////////////////////////////////////////////////////////

// Cambio del color del pawn segun el playercontroller que lo controle.
void ASCharacter::OnRep_PlayerColor()
{
	if(MeshID)
	{
		MeshID->SetVectorParameterValue(FName("BodyColor"), PlayerColor);
	}
}

// Cuando la energia del cliente cambia pero no ha muerto.-
void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health<=0.0f && !bDied)
	{
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(10);
	}
}
// networking --> 
//Mantenemos actualizadas las variables en el cliente usando la macro DOREPLIFETIME
void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, SecondaryWeapon);
	DOREPLIFETIME(ASCharacter, PlayerColor);
	DOREPLIFETIME(ASCharacter, bDied);
}