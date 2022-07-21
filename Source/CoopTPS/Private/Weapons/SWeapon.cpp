// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/CoopPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopTPS.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

ASWeapon::ASWeapon()
{
	PrimaryActorTick.bCanEverTick =false;
	
 	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bReplicates =true;
	SetReplicatingMovement(true);
	bNetUseOwnerRelevancy = true;
	// Ojo con esto que nos estamos cargando la posibilidad de ajuste de epic.
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ASWeapon,CurrentAmmoInBackpack,COND_OwnerOnly);
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Overlap);
		SphereComp->OnComponentBeginOverlap.AddDynamic(this,&ASWeapon::OnSphereOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this,&ASWeapon::OnSphereEndOverlap);
	}
	if(WeaponFXConfig.ReloadMontage)
	{
		WeaponConfig.ReloadTime = WeaponFXConfig.ReloadMontage->GetSectionLength(0);
	}	
	if(WeaponConfig.FireRate >0)
	{
		TimeBetweenShots = UKismetMathLibrary::SafeDivide(60, WeaponConfig.FireRate); // 10 balas por segundo;
	}
}

void ASWeapon::Fire()
{
	if(!HasAuthority())
	{
		ServerFire();
	}	
}

void ASWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		StopFire();
		PlayerController = nullptr;
		const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
		MeshComp->DetachFromComponent(DetachRules);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetEnableGravity(true);
		MeshComp->AddImpulse(GetActorForwardVector() * -600.0f);
		UE_LOG(LogTemp,Warning,TEXT("NULL OWNER"));
		MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		PlayerController = Cast<ACoopPlayerController>(Cast<APawn>(GetOwner())->GetController());
		if(PlayerController)
		{
			UE_LOG(LogTemp,Warning,TEXT("OWNER %s"),*PlayerController->GetName());
		}
		else
		{
			UE_LOG(LogTemp,Error,TEXT("OWNER FAILED"));
		}
		
	}
}
void ASWeapon::StartReloading()
{
	if(!HaveAmmoInMag() || bIsReloading){return;}
	bIsReloading = true;
	StopFire();
	Reload();
}

bool ASWeapon::IsReloading() const
{
	return bIsReloading;
}
/*
void ASWeapon::OnRep_Reloading()
{
	if(!bIsReloading)
	{
		CalculateAmmo();	
		UpdateAmmoInfoUI();
	}
}
*/
void ASWeapon::Reload()
{
	if(!HasAuthority())
	{
		ServerReload();
	}
	if(WeaponFXConfig.ReloadSFX)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFXConfig.ReloadSFX);
	}
	if(FTimerHandle Th_FinishReload;
	!GetWorldTimerManager().IsTimerActive(Th_FinishReload))
	{
		GetWorldTimerManager().SetTimer(Th_FinishReload,this,&ASWeapon::FinishReloading,WeaponConfig.ReloadTime,false);
	}	
}

void ASWeapon::FinishReloading()
{
	CalculateAmmo();
	bIsReloading = false;
}

void ASWeapon::CalculateAmmo()
{
	const int32 DeltaAmmo = UKismetMathLibrary::Min(CurrentAmmoInBackpack,WeaponConfig.MaxAmmo-CurrentAmmo);
	CurrentAmmo += DeltaAmmo;
	CurrentAmmoInBackpack -= DeltaAmmo;
	UpdateAmmoInfoUI();
}

void ASWeapon::SpendAmmo()
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo -1,0,CurrentAmmoInBackpack);
	if(HasAuthority())
	{
		ClientAmmoUpdate(CurrentAmmo);
		UpdateAmmoInfoUI();	
	}
	else
	{
		++AmmoSequence;
	}
}

void ASWeapon::ClientAmmoUpdate_Implementation(const int32 ServerAmo)
{
	if(HasAuthority()){return;}
	CurrentAmmo = ServerAmo;
	--AmmoSequence;
	CurrentAmmo -= AmmoSequence;
	UpdateAmmoInfoUI();		
}

void ASWeapon::UpdateAmmoInfoUI()
{
	PlayerController = PlayerController == nullptr ?
		Cast<ACoopPlayerController>(Cast<APawn>(GetOwner())->GetController()) : PlayerController;
	//PlayerController = Cast<ACoopPlayerController>(Cast<APawn>(GetOwner())->GetController());
	if(PlayerController)
	{
		PlayerController->UpdateCurrentAmmo(CurrentAmmo,CurrentAmmoInBackpack);
	}
}

void ASWeapon::OnRep_CurrentAmmo()
{
	UpdateAmmoInfoUI();
}

void ASWeapon::EquipWeapon(USceneComponent* MeshComponent, const FName& WeaponSocket)
{
	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetEnableGravity(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->AttachToComponent(MeshComponent,FAttachmentTransformRules::SnapToTargetIncludingScale,WeaponSocket);
	}
	else
	{
		ServerEquipWeapon(MeshComponent,WeaponSocket);
	}
	UpdateAmmoInfoUI();
}

void ASWeapon::DropWeapon()
{
	if(HasAuthority())
	{
		StopFire();
		SetOwner(nullptr);
		PlayerController =nullptr;
		const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
		MeshComp->DetachFromComponent(DetachRules);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetEnableGravity(true);
		MeshComp->AddImpulse(GetActorForwardVector() * -600.0f);
		
		MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		
		if(FTimerHandle Th_CanBePicked; !GetWorldTimerManager().IsTimerActive(Th_CanBePicked))
		{
			GetWorldTimerManager().SetTimer(Th_CanBePicked,this,&ASWeapon::SetPickable,2.0f,false);
		}
	}
}

const FHUDData& ASWeapon::GetCrosshairData() const
{
	return CrossHairData;
}

UAnimMontage* ASWeapon::GetFireMontage() const
{
	return WeaponFXConfig.FireMontage;
}

UAnimMontage* ASWeapon::GetReloadMontage() const
{
	return WeaponFXConfig.ReloadMontage;
}

FTransform ASWeapon::GetWeaponHandle() const
{
	if(const auto Socket = MeshComp->GetSocketByName(FName("HandleSocket")); IsValid(Socket))
	{
		return MeshComp->GetSocketTransform(FName("HandleSocket"),RTS_World);
	}
	return FTransform{};
}

FName ASWeapon::GetWeaponName() const
{
	return WeaponConfig.WeaponName;
}

int32 ASWeapon::GetWeaponCurrentAmmo() const
{
	return CurrentAmmo;
}

FName ASWeapon::GetWeaponsName() const
{
	return WeaponConfig.WeaponName;
}

int32 ASWeapon::GetAmmoInBackpack() const
{
	return CurrentAmmoInBackpack;
}

int32 ASWeapon::GetWeaponMaxAmmo() const
{
	return WeaponConfig.MaxAmmo;
}

void ASWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ASWeapon::StartFire()
{
	if(CurrentAmmo <=0 || bIsReloading){return;}
	if (!GetWorldTimerManager().IsTimerActive(Th_TimeBetweenShots))
	{
		const float FireDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->GetTimeSeconds(),0.0f); 
		GetWorldTimerManager().SetTimer(Th_TimeBetweenShots, this, &ASWeapon::Fire,
														TimeBetweenShots, true,FireDelay);
	}
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(Th_TimeBetweenShots);
}

void ASWeapon::PlayImpactFX(const EPhysicalSurface NewSurfaceType, const FVector& ImpactPoint, const FVector& ImpactNormal) const
{
	const FRotator ImpactRotation = ImpactNormal.ToOrientationRotator();
	UParticleSystem* SelectedFX;
	switch (NewSurfaceType)
	{
	case SURFACE_FLESHDEFAULT: //flesh default
	case SURFACE_FLESHVULNERABLE: // flesh headshot
		SelectedFX = WeaponFXConfig.FleshImpactFX;
		break;
	default:
		SelectedFX = WeaponFXConfig.DefaultImpactFX;
		break;
	}
	if (SelectedFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, ImpactPoint,ImpactRotation); //ShotDirection.Rotation()
	}
}

void ASWeapon::PlayShootVfx(const FVector TraceEnd) const
{
	if (WeaponFXConfig.MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(WeaponFXConfig.MuzzleFX, MeshComp, WeaponConfig.MuzzleSocketName);
	}
	const FVector_NetQuantize MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);
	if (UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
														WeaponFXConfig.TracerFX, MuzzleLocation))
	{
		TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
	}
	// Camera shake
	if(const APawn* MyOwner = Cast<APawn>(GetOwner()))
	{
		if(APlayerController* PC = Cast<APlayerController>(MyOwner->GetController()))
		{
			PC->ClientStartCameraShake(WeaponFXConfig.FireCamShake);
		}
	}
}

//call on client | execute on server
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ServerReload_Implementation()
{
	Reload();
}

bool ASWeapon::ServerReload_Validate()
{
	return true;
}

void ASWeapon::ServerEquipWeapon_Implementation(USceneComponent* MeshComponent, const FName& WeaponSocket)
{
	EquipWeapon(MeshComponent,WeaponSocket);
}
/*
void ASWeapon::ServerDropWeapon_Implementation()
{
	DropWeapon();
}*/

// ReSharper disable once CppMemberFunctionMayBeStatic
void ASWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// show widget to the player who is overlapping,
	// OtherActor->ShowWidget(this) -> this function will show the proper widget
	//only for the player who is overlapping.
	if(OtherActor->Implements<UIInputComm>())
	{
		const auto I = Cast<IIInputComm>(OtherActor);
		I->I_SetOverlappingWeapon(this);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ASWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// OtherActor->ShowWidget(null)
}

void ASWeapon::SetPickable() const
{
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetEnableGravity(false);
	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

bool ASWeapon::HaveAmmoInMag() const
{
	return (WeaponConfig.MaxAmmo >0);
}
