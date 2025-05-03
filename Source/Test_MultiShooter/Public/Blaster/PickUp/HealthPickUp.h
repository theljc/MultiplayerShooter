// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/PickUp/PickUp.h"
#include "HealthPickUp.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()
public:
	AHealthPickUp();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere, Category=Health)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere, Category=Health)
	float HealingTime = 5.f;


	
};
