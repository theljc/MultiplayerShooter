// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/HitScanWeapon.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API AShotGun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
	FVector ShotgunTraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets;
	
};
