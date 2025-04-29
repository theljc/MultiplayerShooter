// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class TEST_MULTISHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere)
	float Damage;
	
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere)
    TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> TracerSystem;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TracerSystemComponent;




	
};
