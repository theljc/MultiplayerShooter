// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/GrenadeProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AGrenadeProjectile::AGrenadeProjectile()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
	
}

void AGrenadeProjectile::Destroyed()
{
	ExplodeDamage();
	
	Super::Destroyed();
}

void AGrenadeProjectile::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AGrenadeProjectile::OnBounce);
	
}

void AGrenadeProjectile::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
			);
		
	}
}
