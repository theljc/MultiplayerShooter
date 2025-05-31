// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Weapon/Weapon.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Casing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
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
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
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

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	FTransform SocketTransform = GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
	FVector TraceStart = SocketTransform.GetLocation();
	
	FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 6, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan, true);

	// 除以 Size 为了防止浮点数溢出
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
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
		// 不能捡起对方阵营的旗帜
		if (WeaponTypes == EWeaponTypes::EWT_Flag and BlasterCharacter->GetTeam() != Team) return;
		// 处于旗帜状态下不能捡起武器
		if (BlasterCharacter->IsHoldingFlag()) return;
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
		if (WeaponTypes == EWeaponTypes::EWT_Flag and BlasterCharacter->GetTeam() != Team) return;
		if (BlasterCharacter->IsHoldingFlag()) return;
		// 在服务器上调用的，传入空指针隐藏 UI
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = bPingTooHigh;
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
	ClientAddAmmo(AmmoAmount);
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;

	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(GetOwner())) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent() && IsAmmoFull())
	{
		BlasterOwnerCharacter->GetCombatComponent()->JumpToShotGunEnd();
	}
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
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(Owner)) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetWeapon() && BlasterOwnerCharacter->GetWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
	
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::SetWeaponState(EWeaponState NewWeaponState)
{
	// 武器状态改变时复制到其它客户端
	WeaponState = NewWeaponState;
	OnWeaponStateSet();
	
}

void AWeapon::OnWeaponStateSet()
{
	// 在服务器端修改武器的状态，才能同步到客户端
	switch (WeaponState)
	{
	case EWeaponState::EWC_Equipped:
		EquipWeapon();
		break;
	case EWeaponState::EWC_EquipSecondary:
		EquipSecondaryWeapon();
		break;
	case EWeaponState::EWC_Dropped:
		DropWeapon();
		break;
	}
	
}

void AWeapon::EquipWeapon()
{
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
	EnableCustomDepth(false);

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(GetOwner())) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter and bUseServerSideRewind)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller)) : BlasterOwnerPlayerController;
		// 判断服务器上委托是否已绑定
		if (BlasterOwnerPlayerController and HasAuthority() and !BlasterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
	
}

void AWeapon::DropWeapon()
{
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
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(GetOwner())) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter and bUseServerSideRewind)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller)) : BlasterOwnerPlayerController;
		// 判断服务器上委托是否已绑定
		if (BlasterOwnerPlayerController and HasAuthority() and !BlasterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
	
}

bool AWeapon::IsAmmoEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsAmmoFull()
{
	return Ammo == MagCapacity;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
	
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

void AWeapon::EquipSecondaryWeapon()
{
	ShowPickUpWidget(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponTypes == EWeaponTypes::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(GetOwner())) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter and bUseServerSideRewind)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller)) : BlasterOwnerPlayerController;
		// 判断服务器上委托是否已绑定
		if (BlasterOwnerPlayerController and HasAuthority() and !BlasterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
	
}

