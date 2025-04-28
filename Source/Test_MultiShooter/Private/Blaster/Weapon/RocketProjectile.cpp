// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/RocketProjectile.h"

#include "Kismet/GameplayStatics.h"

ARocketProjectile::ARocketProjectile()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController  = FiringPawn->Controller;
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				500.f,
				200.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
				);
		}
	}
	
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, HitResult);
}
