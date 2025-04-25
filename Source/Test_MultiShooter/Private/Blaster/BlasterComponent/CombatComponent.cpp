// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterComponent/CombatComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
	
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		// FHitResult HitResult;
		// TraceUnderCrossHair(HitResult);
		Server_Fire(HitTarget);
		StartFireTimer();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed and EquipWeapon)
	{
		Fire();
	}
	
}

void UCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquipWeapon == nullptr) return;

	if (Character)
	{
		// 服务器端执行，并且广播到所有客户端
		NetMulticast_Fire(TraceHitTarget);
	}
}

void UCombatComponent::NetMulticast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	Character->PlayFireMontage(bAiming);
	EquipWeapon->Fire(TraceHitTarget);
}

void UCombatComponent::TraceUnderCrossHair(FHitResult& HitResult)
{
	FVector2D ViewPortSize;
	if (GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CrossHairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;
	bool bDeproject = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation,
		CrossHairWorldPosition,
		CrossHairWorldDirection
	);

	if (bDeproject)
	{

		FVector Start = CrossHairWorldPosition;
		
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrossHairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrossHairWorldDirection * TRACE_LENGTH;
		
		GetWorld()->LineTraceSingleByChannel(HitResult,
			Start, End,
			ECC_Visibility);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
		
		if (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractCrosshair_Interface>())
		{
			HUDPackage.CrossHairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrossHairsColor = FLinearColor::White;
		}
		
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	
	CharacterPlayerController = CharacterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : CharacterPlayerController;
	if (CharacterPlayerController)
	{
		CharacterHUD = CharacterHUD == nullptr ? Cast<ABlasterHUD>(CharacterPlayerController->GetHUD()) : CharacterHUD;
		if (CharacterHUD)
		{
			if (EquipWeapon)
			{
				HUDPackage.CrossHairs = EquipWeapon->CrossHairs;
			}
			else
			{
				HUDPackage.CrossHairs = nullptr;
			}

			CharacterHUD->SetHUDPackage(HUDPackage);
		}
	}

}

bool UCombatComponent::CanFire()
{
	if (EquipWeapon == nullptr) return false;
	return !EquipWeapon->IsAmmoEmpty() || !bCanFire;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquipWeapon) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquipWeapon->GetZoomedFOV(), DeltaTime, EquipWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);
	}

	if (Character and Character->GetCamera())
	{
		Character->GetCamera()->SetFieldOfView(CurrentFOV);
	}
	
}

void UCombatComponent::StartFireTimer()
{
	if (EquipWeapon == nullptr and Character == nullptr) return;
	
	Character->GetWorldTimerManager().SetTimer(FireTimer,
		this,
		&UCombatComponent::FireTimerFinish,
		EquipWeapon->FireDelay);
	
}

void UCombatComponent::FireTimerFinish()
{
	if (EquipWeapon == nullptr) return;

	bCanFire = true;
	if (bFireButtonPressed and EquipWeapon->bAutoFire)
	{
		Fire();
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (Character and Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrossHair(HitResult);
		HitTarget = HitResult.ImpactPoint;
			
		SetHUDCrosshairs(DeltaTime);

		InterpFOV(DeltaTime);
		
	}
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquipWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	
}

void UCombatComponent::EquippedWeapon(AWeapon* WeaponToEquipped)
{
	if (Character == nullptr || WeaponToEquipped == nullptr) return;
	if (EquipWeapon)
	{
		EquipWeapon->Dropped();
	}
	
	EquipWeapon = WeaponToEquipped;
	EquipWeapon->SetWeaponState(EWeaponState::EWC_Equipped);
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName("WeaponSocket"));
	if (WeaponSocket)
	{
		WeaponSocket->AttachActor(EquipWeapon, Character->GetMesh());
	}
	EquipWeapon->SetOwner(Character);
	EquipWeapon->SetHUDAmmo();
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquipWeapon && Character)
	{
		// 由于复制到客户端时无法确定 WeaponState 和 Attach 哪个先执行
		// 所以要确保 WeaponState 在 Attach 之前被设置，因为模拟物理的 Actor 不能被 Attach
		EquipWeapon->SetWeaponState(EWeaponState::EWC_Equipped);
		const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName("WeaponSocket"));
		if (WeaponSocket)
		{
			WeaponSocket->AttachActor(EquipWeapon, Character->GetMesh());
		}
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	// 调用 RPC 函数通知服务器修改客户端的变量
	Server_Aiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	
}

void UCombatComponent::Server_Aiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	
}

