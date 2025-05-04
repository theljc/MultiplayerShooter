// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;
class USphereComponent;

UCLASS()
class TEST_MULTISHOOTER_API APickUp : public AActor
{
	GENERATED_BODY()

public:
	APickUp();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PickUpMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> PickUpEffectComponent;

	UPROPERTY(EditAnywhere, Category=Health)
	TObjectPtr<UNiagaraSystem> PickUpEffect;

	// 如果生成时角色已经站在生成点上，会导致碰撞球触发后直接销毁，还没来得及绑定到函数
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> PickUpSound;
	
};
