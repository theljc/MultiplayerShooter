// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/ProjectileWeapon.h"

#include "Blaster/Weapon/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// 武器的开火行为只在服务器上执行
	if (!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MeshSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MeshSocket)
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
			
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParameters);
			}
		}
	}
	
}
