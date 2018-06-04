// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWeapon.h"
#include "SProjectile.h"


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

	// camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	//ThrowPosition = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowPosComp"));
	//ThrowPosition->SetRelativeLocation(FVector(-30.0f, 9.0f, 110.0f));

	// Default FOV 
	
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;
	WeaponSocketName = "WeaponSocket";
	GrenadeSocketName = "GrenadeSocket";
	InitialVelocity = FVector(1000.0f, 0.0f, 1000.0f);
	bIsGranadaMode = false;
	LaunchDistance = 20.0f;
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
		const FRotator SpawnRotation = GetControlRotation();
		const FVector SpawnLocation = GetMesh()->GetSocketLocation(GrenadeSocketName);
		const FVector SpawnScale = FVector(0.1f);
		UWorld* World = GetWorld();
		//FActorSpawnParameters GranadaSpawnParams;
		//GranadaSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		ASProjectile* Grenade = World->SpawnActorDeferred<ASProjectile>(GranadaClass, FTransform(SpawnRotation, SpawnLocation));
		if (Grenade)
		{
		Grenade->InitialLocalVelocity = InitialVelocity;
		//Grenade->ProjectileComp->SetVelocityInLocalSpace(InitialVelocity);
		UGameplayStatics::FinishSpawningActor(Grenade, FTransform(SpawnRotation, SpawnLocation, SpawnScale));
		}
		
	}
}

void ASCharacter::StartThrow() // TODO Continuar!
{
	bIsGranadaMode = true;	
}

void ASCharacter::StopThrow()
{
	bIsGranadaMode = false;
	LaunchDistance = 0.20f;
	Throw();
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// in grenade mode
	if(bIsGranadaMode)
	{
		float adding = 20.0f;
		LaunchDistance = FMath::Clamp(LaunchDistance + adding, 1.0f, 1000.0f);
		InitialVelocity = FVector(LaunchDistance, 0.0f, LaunchDistance);
		UE_LOG(LogTemp, Warning, TEXT("%f"),LaunchDistance);
	}

	// is true : first - false : second
	float TargetFOV = bIsZoomed ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}