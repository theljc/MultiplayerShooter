// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterComponent/CombatComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquipWeapon);
	
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
	
}

