// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class URocketMovementComponent;
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
	virtual void Destroyed() override;

	/** 
	* Used with server-side rewind
	*/

	bool bUseServerSideRewind = false;
	// 相比 Vector ，对网络复制做了优化
	FVector_NetQuantize TraceStart;
	// 精度更高的 NetQuantize 类型，保留了小数点后两位
	FVector_NetQuantize100 InitialVelocity;
	
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	UPROPERTY(EditAnywhere)
	float Damage = 5.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URocketMovementComponent> RocketMovementComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();


	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere)
    TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> TracerSystem;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TracerSystemComponent;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
};
