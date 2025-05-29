// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/ProjectileWeapon.h"

#include "Blaster/Weapon/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// 武器的开火行为只在服务器上执行
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MeshSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MeshSocket and World)
	{
		FTransform SocketTransform = MeshSocket->GetSocketTransform(GetWeaponMesh());
		// 武器插槽位置到目标位置的距离
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 武器插槽位置到目标位置的方向
		FRotator TargetRotation = ToTarget.Rotation();
		
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;

			AProjectile* SpawnedProjectile = nullptr;

			if (bUseServerSideRewind) // 该武器使用服务器倒带的情况
			{
				if (InstigatorPawn->HasAuthority())
				{
					if (InstigatorPawn->IsLocallyControlled()) // 服务器本地玩家
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;
						SpawnedProjectile->HeadShotDamage = HeadShotDamage;
					}
					else // 服务器上的其他玩家
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
					}
				}
				else
				{
					if (InstigatorPawn->IsLocallyControlled()) // 客户端本地玩家
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					}
					else // 客户端上的其他玩家
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false;
					}
				}
			}
			else // 不使用服务器倒带的情况
			{
				if (InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
			
		}
	}
	
}
