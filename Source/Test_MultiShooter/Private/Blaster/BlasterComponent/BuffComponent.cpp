// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterComponent/BuffComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::Heal(float HealTime, float HealingAmount)
{
	bHealing = true;

	HealRate = HealingAmount / HealTime;
	HealAmount += HealingAmount;
	
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (BlasterCharacter == nullptr) return;

	BlasterCharacter->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime);

	if (BlasterCharacter->GetCharacterMovement())
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (BlasterCharacter == nullptr) return;

	BlasterCharacter->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime);

	if (BlasterCharacter->GetCharacterMovement())
	{
		BlasterCharacter->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::BuffShield(float ShieldAmount, float ShieldReplenishTime)
{
	bShield = true;

	ShieldReplenishRate = ShieldAmount / ShieldReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
	
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (BlasterCharacter == nullptr or BlasterCharacter->IsElimed() or !bShield) return;
	
	float ShieldThisFrame = ShieldReplenishRate * DeltaTime;
	BlasterCharacter->SetShield(FMath::Clamp(BlasterCharacter->GetShield() + ShieldThisFrame, 0, BlasterCharacter->GetMaxShield()));
	BlasterCharacter->UpdateHUDShield();

	ShieldReplenishAmount -= ShieldThisFrame;

	if (ShieldReplenishAmount <= 0.f or BlasterCharacter->GetShield() >= BlasterCharacter->GetMaxShield())
	{
		bShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::ResetSpeeds()
{
	if (BlasterCharacter == nullptr or BlasterCharacter->GetCharacterMovement() == nullptr) return;

	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);

}

void UBuffComponent::ResetJump()
{
	if (BlasterCharacter == nullptr or BlasterCharacter->GetCharacterMovement() == nullptr) return;
	BlasterCharacter->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	BlasterCharacter->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (BlasterCharacter == nullptr or BlasterCharacter->IsElimed() or !bHealing) return;
	
	float HealThisFrame = HealRate * DeltaTime;
	BlasterCharacter->SetHealth(FMath::Clamp(BlasterCharacter->GetHealth() + HealThisFrame, 0, BlasterCharacter->GetMaxHealth()));
	BlasterCharacter->UpdateHUDHealth();

	HealAmount -= HealThisFrame;

	if (HealAmount <= 0.f or BlasterCharacter->GetHealth() >= BlasterCharacter->GetMaxHealth())
	{
		bHealing = false;
		HealAmount = 0.f;
	}
	
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
	
}



