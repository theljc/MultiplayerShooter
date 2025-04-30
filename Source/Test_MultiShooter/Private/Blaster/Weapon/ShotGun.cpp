// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/ShotGun.h"
#include "Blaster/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

void AShotGun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	FTransform SocketTransform = GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
	FVector Start = SocketTransform.GetLocation();

	TMap<ABlasterCharacter*, uint32> HitMap;
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		FHitResult HitResult;

		WeaponTraceHit(Start, HitTarget, HitResult);
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
		if (BlasterCharacter and HasAuthority() and InstigatorController)
		{
			if (HitMap.Contains(BlasterCharacter))
			{
				HitMap[BlasterCharacter]++;
			}
			else
			{
				HitMap.Emplace(BlasterCharacter, 1);
			}

			if (ImpactParticles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                	GetWorld(),
                	ImpactParticles,
                	HitResult.ImpactPoint,
                	HitResult.ImpactNormal.Rotation()
                );
            }
            
            if (HitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                	this,
                	HitSound,
                	HitResult.ImpactPoint
                );
            }
			
		}
	}

	for (auto HitPair : HitMap)
	{
		if (HitPair.Key and HasAuthority() and InstigatorController)
		{
			UGameplayStatics::ApplyDamage(HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
		}
	}

	// UWorld* World = GetWorld();
	// if (World)
	// {
	// 	// FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
	//
	// 	World->LineTraceSingleByChannel(
	// 		HitResult,
	// 		Start,
	// 		End,
	// 		ECollisionChannel::ECC_Visibility
	// 	);
	// 	FVector BeamEnd = End;
	// 	if (HitResult.bBlockingHit)
	// 	{
	// 		BeamEnd = HitResult.ImpactPoint;
	// 	}
	// 	else
	// 	{
	// 		HitResult.ImpactPoint = End;
	// 	}
	// }
	
	// WeaponTraceHit(Start, HitTarget, HitResult);
	


	
}

FVector AShotGun::ShotgunTraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 6, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan, true);

	// 除以 Size 为了防止浮点数溢出
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
