// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/Weapon.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Casing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pick Up Widget"));
	PickUpWidget->SetupAttachment(RootComponent);
	
}

void AWeapon::ShowPickUpWidget(bool bShow)
{
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShow);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
	
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter)
	{
		// 在服务器上调用的
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		// 在服务器上调用的，传入空指针隐藏 UI
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(GetOwner())) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller)) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController)
		{
			BlasterOwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoAmount)
{
	// Ammo 改变时通过 OnRep 函数更新客户端 HUD
	Ammo = FMath::Clamp(Ammo + AmmoAmount, 0, MagCapacity);
	// 更新服务器上的 HUD
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerPlayerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
	
}

void AWeapon::OnRep_WeaponState()
{
	// 根据当前武器的状态来决定是否显示 UI
	switch (WeaponState)
	{
	case EWeaponState::EWC_Equipped:
		ShowPickUpWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponTypes == EWeaponTypes::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
		break;
	case EWeaponState::EWC_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState NewWeaponState)
{
	// 武器状态改变时复制到其它客户端
	WeaponState = NewWeaponState;

	// 在服务器端修改武器的状态，才能同步到客户端
	switch (WeaponState)
	{
	case EWeaponState::EWC_Equipped:
		ShowPickUpWidget(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponTypes == EWeaponTypes::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
		break;
	case EWeaponState::EWC_Dropped:
		if (HasAuthority())
		{
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		break;
	}
	
}

bool AWeapon::IsAmmoEmpty()
{
	return Ammo <= 0;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
	
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		FActorSpawnParameters SpawnParams;
		
		const USkeletalMeshSocket* MeshSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		FTransform SocketTransform = MeshSocket->GetSocketTransform(WeaponMesh);
		
		GetWorld()->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator(), SpawnParams);
	}

	SpendRound();
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWC_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerPlayerController = nullptr;
}

