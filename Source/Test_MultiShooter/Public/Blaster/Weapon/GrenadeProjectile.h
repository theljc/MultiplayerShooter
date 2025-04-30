// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Projectile.h"
#include "GrenadeProjectile.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API AGrenadeProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	AGrenadeProjectile();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> BounceSound;
	
};
