// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterComponent/CombatComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
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


void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquipWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
	
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrossHair(HitResult);
		Server_Fire(HitResult.ImpactPoint);
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
		FVector End = Start + CrossHairWorldDirection * TRACE_LENGTH;
		
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrossHairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		GetWorld()->LineTraceSingleByChannel(HitResult,
			Start, End,
			ECC_Visibility);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
		
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	FHUDPackage HUDPackage;

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

	// if (CharacterPlayerController == nullptr)
	// {
	// 	CharacterPlayerController = Cast<ABlasterPlayerController>(Character->Controller);
	// 	if (CharacterPlayerController)
	// 	{
	// 		if (CharacterHUD == nullptr)
	// 		{
	// 			CharacterHUD = Cast<ABlasterHUD>(CharacterPlayerController->GetHUD());
	// 			if (CharacterHUD)
	// 			{
	// 				if (EquipWeapon)
	// 				{
	// 					if (CharacterHUD)
	// 					{
	// 						HUDPackage.CrossHairs = EquipWeapon->CrossHairs;
	// 					}
	// 					else
	// 					{
	// 						HUDPackage.CrossHairs = nullptr;
	// 					}
	// 				}
	// 	
	// 				
	// 			}
	// 		}
	// 	}
	// }
	



	
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshairs(DeltaTime);
	
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

	EquipWeapon = WeaponToEquipped;
	EquipWeapon->SetWeaponState(EWeaponState::EWC_Equipped);
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName("WeaponSocket"));
	if (WeaponSocket)
	{
		WeaponSocket->AttachActor(EquipWeapon, Character->GetMesh());
	}
	EquipWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
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

