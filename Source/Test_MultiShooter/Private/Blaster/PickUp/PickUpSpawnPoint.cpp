// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PickUp/PickUpSpawnPoint.h"

#include "Blaster/PickUp/PickUp.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	StartSpawnPickupTimer((AActor*)nullptr);
	
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUpSpawnPoint::SpawnPickUp()
{
	int32 NumPickUpClass = PickUpClass.Num();
	if (NumPickUpClass > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickUpClass - 1);
		SpawnPickup = GetWorld()->SpawnActor<APickUp>(PickUpClass[Selection], GetActorTransform());
	}

	if (HasAuthority() and SpawnPickup)
	{
		SpawnPickup->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::StartSpawnPickupTimer);
	}
	
}

void APickUpSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
	
}

void APickUpSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
 	GetWorldTimerManager().SetTimer(
 		SpawnPickupTimer,
 		this,
 		&APickUpSpawnPoint::SpawnPickupTimerFinished,
 		SpawnTime
 		);

}

