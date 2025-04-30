// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	virtual void Fire(const FVector& HitTarget) override;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> ImpactParticles;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> HitSound;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> BeamParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> FireSound;
	
};
