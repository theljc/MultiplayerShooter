// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/ShotGun.h"
#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	FTransform SocketTransform = GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
	FVector Start = SocketTransform.GetLocation();

	TMap<ABlasterCharacter*, uint32> HitMap;
	TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
	
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter)
		{
			const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
			if (bHeadShot)
			{
				if (HeadShotHitMap.Contains(BlasterCharacter)) HeadShotHitMap[BlasterCharacter]++;
				else HeadShotHitMap.Emplace(BlasterCharacter, 1);
			}
			else
			{
				if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter]++;
				else HitMap.Emplace(BlasterCharacter, 1);
			}
		}
		
	}

	TArray<ABlasterCharacter*> HitCharacters;

	// Maps Character hit to total damage
	TMap<ABlasterCharacter*, float> DamageMap;
	
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key and HasAuthority() and InstigatorController)
		{
			DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
			HitCharacters.AddUnique(HitPair.Key);
		}
	}

	for (auto HeadShotHitPair : HeadShotHitMap)
	{
		if (HeadShotHitPair.Key)
		{
			if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
			else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

			HitCharacters.AddUnique(HeadShotHitPair.Key);
		}
	}

	for (auto DamagePair : DamageMap)
	{
		if (DamagePair.Key && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(DamagePair.Key,
					Damage * DamagePair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
					);
			}
		}
	}
	
	// 客户端上霰弹枪开火时应用服务器滞后补偿
	if (!HasAuthority() && bUseServerSideRewind)
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(OwnerPawn)) : BlasterOwnerCharacter;
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(InstigatorController)) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensationComponent() && BlasterOwnerCharacter->IsLocallyControlled())
		{
			BlasterOwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
				HitCharacters,
				Start,
				HitTargets,
				BlasterOwnerPlayerController->GetServerTime() - BlasterOwnerPlayerController->SingleTripTime
			);
		}
	}
	
}

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	FTransform SocketTransform = GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
	FVector TraceStart = SocketTransform.GetLocation();
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
		FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		
		HitTargets.Add(ToEndLoc);
	}

}
