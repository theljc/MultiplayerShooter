// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/RocketProjectile.h"

#include "Blaster/Weapon/RocketMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ARocketProjectile::ARocketProjectile()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(FName("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
	
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
				200.f,
				500.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
				);
		}
	}

	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AProjectile::DestroyTimerFinished,
		3.f);

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	
	StaticMesh->SetVisibility(false);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	
}

void ARocketProjectile::Destroyed()
{
	
}

