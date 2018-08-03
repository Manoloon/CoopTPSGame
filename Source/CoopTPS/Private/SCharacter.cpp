// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWeapon.h"
#include "SProjectile.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystem.h" // beam test
#include "Particles/ParticleSystemComponent.h"
#include "ParticleDefinitions.h"
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

	// this line is for crouch
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore); // de esta manera la capsula no bloqueara nuestra arma.

	// camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// beamcomponent
	BeamComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(SpringArmComp);
	BeamComp->bAutoActivate = false;

	BeamEndPointDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BeamEndPointDecal"));
	BeamEndPointDecal->SetVisibility(false);

	// health Component
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	// Default FOV 
	
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;
	WeaponSocketName = "WeaponSocket";
	GrenadeSocketName = "GrenadeSocket";
	bIsGranadaMode = false;
	LaunchDistance = 100.0f;
	PathLifeTime = 5.0f;
	TimeInterval = 0.05;
	Gravity = FVector(0.0f, 0.0f, -980.0f);
	SpawnScale = FVector(0.1f);
	bDied = false;
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

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	// health start
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	if(Role == ROLE_Authority)
	{
		// spawn initial weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
	}
	
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

void ASCharacter::Throw()
{
	if (GranadaClass)
	{
		ClearBeam();
		UWorld* World = GetWorld();
		ASProjectile* Grenade = World->SpawnActorDeferred<ASProjectile>(GranadaClass, FTransform(SpawnRotation, StartLocation));
		
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
for(auto Beam = BeamArray.CreateIterator(); Beam; Beam++)
{
	(*Beam)->DestroyComponent();
}
	BeamArray.Empty();
}

void ASCharacter::AddNewBeam(FVector Point1, FVector Point2)
{
	
	BeamComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamFX, Point1, FRotator::ZeroRotator, true);
	BeamArray.Add(BeamComp);

		BeamComp->SetBeamSourcePoint(0, Point1, 0);
		BeamComp->SetBeamTargetPoint(0, Point2, 0);
}


void ASCharacter::GetSegmentAtTime(FVector LocalStartLocation, FVector LocalInitialVelocity, FVector LocalGravity, float LocalTime1, float LocalTime2, FVector &OutPoint1, FVector &OutPoint2)
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
		
		const UWorld* World = GetWorld();
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
			if (World->LineTraceSingleByChannel(Hit, Point1, Point2, ECC_Visibility, QueryParams))
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

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// in grenade mode
	if(bIsGranadaMode)
	{
		float adding = 10.0f;
		LaunchDistance = FMath::Clamp(LaunchDistance + adding, 1.0f, 1000.0f);
		InitialLocalVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance);
		DrawingTrajectory();
	}

	// is true : first - false : second
	float TargetFOV = bIsZoomed ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

// networking
void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// esto hace que se replique dicha variable a todos nuestros clientes.
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}