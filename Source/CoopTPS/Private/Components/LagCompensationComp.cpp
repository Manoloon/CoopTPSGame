// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/LagCompensationComp.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Entities/SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// console debugging
static bool DebugServerRewind = false;
FAutoConsoleVariableRef CVarDebugServerRewind(TEXT("Coop.DebugServerRewind"), DebugServerRewind,
											TEXT("Server Rewind"), ECVF_Cheat);

ULagCompensationComp::ULagCompensationComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComp::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComp::SaveFramePackage(FFramePackage& NewFramePackage) const
{
	if(const auto Pawn = Cast<ASCharacter>(GetOwner());
		IsValid(Pawn))
	{
		NewFramePackage.Time = GetWorld()->GetTimeSeconds();
		for (const auto& HitBox : Pawn->GetRewindHitBoxes())
		{
			FHitBoxInfo BoxInformation;
			BoxInformation.Location = HitBox.Value->GetComponentLocation();
			BoxInformation.Rotation = HitBox.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitBox.Value->GetScaledBoxExtent();
			NewFramePackage.HitBoxInfoContainer.Add(HitBox.Key, BoxInformation);
		}
	}
}

void ULagCompensationComp::SaveFramePackage()
{
	if(GetOwner() && !GetOwner()->HasAuthority())
	{
		return;
	}
	if(FrameHistory.Num() <=1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		if(const float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
			HistoryLength > MaxRecordedTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		if(DebugServerRewind)
		{
			ShowFramePackage(ThisFrame);
		}		
	}
}

void ULagCompensationComp::MoveCollisionBoxes(const ASCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr){return;}
	for (auto& HitBoxPair : HitCharacter->GetRewindHitBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfoContainer[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfoContainer[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfoContainer[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComp::ResetHitBoxes(const ASCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr){return;}
	for (auto& HitBoxPair : HitCharacter->GetRewindHitBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfoContainer[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfoContainer[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfoContainer[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComp::EnablePawnMeshCollision(const ASCharacter* HitCharacter, const ECollisionEnabled::Type CollisionEnable)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnable);
	}
}

void ULagCompensationComp::CacheBoxPosition(const ASCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr){return;}
	for(auto& HitBoxPair : HitCharacter->GetRewindHitBoxes())
	{
		if(HitBoxPair.Value != nullptr)
		{
		FHitBoxInfo BoxInfo;
		BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
		BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
		OutFramePackage.HitBoxInfoContainer.Add(HitBoxPair.Key,BoxInfo);
		}
	}
}

FServerSideRewindResults ULagCompensationComp::ConfirmHit(const FFramePackage& Package, ASCharacter* HitCharacter,
                                                          const FVector_NetQuantize& StartTrace, const FVector_NetQuantize& HitLocation) const
{
	if(HitCharacter == nullptr){return FServerSideRewindResults();}
	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter,CurrentFrame);
	MoveCollisionBoxes(HitCharacter,Package);
	EnablePawnMeshCollision(HitCharacter,ECollisionEnabled::NoCollision);
	
	//enable collision for head box first
	const auto& RewindBoxes = HitCharacter->GetRewindHitBoxes();
	UBoxComponent* HeadBox = RewindBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

	const FVector EndTrace = StartTrace + (HitLocation - StartTrace) * 1.25f;
	if(GetWorld())
	{
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult,StartTrace,EndTrace,ECC_Visibility);
		// HEADSHOT CONFIRM
		if(HitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnablePawnMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResults{true,true};
		}
		for(auto& HitBoxPair : RewindBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
			}
		}
		GetWorld()->LineTraceSingleByChannel(HitResult,StartTrace,EndTrace,ECC_Visibility);
		if(HitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnablePawnMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResults{true,false};
		}
	}
	ResetHitBoxes(HitCharacter,CurrentFrame);
	EnablePawnMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResults{false,false};
}

FFramePackage ULagCompensationComp::FramesInterpolation(const FFramePackage& OlderFrame,
                                                        const FFramePackage& YoungerFrame, float HitTime) const
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime-OlderFrame.Time)/Distance,0.f,1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	for(auto& YoungerPair : YoungerFrame.HitBoxInfoContainer)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FHitBoxInfo& OlderBox = OlderFrame.HitBoxInfoContainer[BoxInfoName];
		const FHitBoxInfo& YoungerBox = YoungerFrame.HitBoxInfoContainer[BoxInfoName];
		FHitBoxInfo InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location,YoungerBox.Location,
																1.f,InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation,YoungerBox.Rotation,
																1.f,InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfoContainer.Add(BoxInfoName,InterpBoxInfo);
	}
	return InterpFramePackage;
}

void ULagCompensationComp::ShowFramePackage(const FFramePackage& FramePackage) const
{
	for(auto& BoxInfo : FramePackage.HitBoxInfoContainer)
	{
		DrawDebugBox(GetWorld(),BoxInfo.Value.Location,
									BoxInfo.Value.BoxExtent,
							FQuat(BoxInfo.Value.Rotation),
									FColor::Blue,false,4.0f);
	}
}

FServerSideRewindResults ULagCompensationComp::ServerSideRewind(ASCharacter* HitCharacter,
                                                                const FVector_NetQuantize& TraceStart,
                                                                const FVector_NetQuantize& HitLocation, float HitTime) const
{
	if(HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr)
			{
		return FServerSideRewindResults();
			}	
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	if(OldestHistoryTime > HitTime)
	{
		return FServerSideRewindResults();
	}
	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate=false;
	}
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate=false;
	}
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime)
	{
		if(Older->GetNextNode() == nullptr){break;}
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if(Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate=false;
	}
	if(bShouldInterpolate)
	{
		FrameToCheck = FramesInterpolation(Older->GetValue(),Younger->GetValue(),HitTime);
	}
	return ConfirmHit(FrameToCheck,HitCharacter,TraceStart,HitLocation);
}

void ULagCompensationComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
}

void ULagCompensationComp::ServerScoreRequest_Implementation(AActor* HitCharacter, const FVector_NetQuantize& StartTrace,
                                                             const FVector_NetQuantize& HitLocation, float HitTime,
															AActor* DamageCauser, float DamageValue,
															const TSubclassOf<UDamageType> DamageType)
{
	if(const auto HitActor = Cast<ASCharacter>(HitCharacter))
	{
		const FServerSideRewindResults Confirm = ServerSideRewind(HitActor,StartTrace,HitLocation,HitTime);
		if(HitActor && GetOwner() && Confirm.bHitConfirmed)
		{
			UGameplayStatics::ApplyDamage(HitActor,DamageValue,GetOwner()->GetInstigatorController(),
			DamageCauser,DamageType);
		}
	}
}

