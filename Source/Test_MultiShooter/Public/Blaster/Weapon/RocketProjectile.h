// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Projectile.h"
#include "RocketProjectile.generated.h"

class URocketMovementComponent;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ARocketProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	ARocketProjectile();
	
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URocketMovementComponent> RocketMovementComponent;
	
};
