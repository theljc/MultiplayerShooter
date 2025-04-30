// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Test_MultiShooter/Test_MultiShooter.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);


	
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();

	if (TracerSystem)
	{
		TracerSystemComponent = UGameplayStatics::SpawnEmitterAttached(
			TracerSystem,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition
			);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
	
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	
	if (ImpactParticle)
    {
    	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
    }

    if (ImpactSound)
    {
    	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString::Printf(TEXT("%s"), *OtherActor->GetName()));

	Destroy();
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
	DestroyTimerHandle,
	this,
	&AProjectile::DestroyTimerFinished,
	DestroyTime
	);
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // BaseDamage
				10.f, // MinimumDamage
				GetActorLocation(), // Origin
				DamageInnerRadius, // DamageInnerRadius
				DamageOuterRadius, // DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this, // DamageCauser
				FiringController // InstigatorController
			);
		}
	}
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

