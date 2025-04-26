// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Character/BlasterAnimInstance.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
		return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAcceleration = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bIsAiming =  BlasterCharacter->IsAiming();
	TurnInPlace = BlasterCharacter->GetTurnInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimed = BlasterCharacter->IsElimed();
	bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffset = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;

	// 计算角色速度方向和控制器方向的差量，用于判断移动方向
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	// 对 rotation 插值，平滑过渡各个动画，RInterpTo 会沿着最短路径插值，从 -180 度直接到 180 度
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	// 根据上一帧和这一帧的旋转的差量，计算 Lean 倾斜度
	LastFrameCharacterRotator = CharacterRotator;
	CharacterRotator = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotator, LastFrameCharacterRotator);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// 获得角色类中计算好的瞄准偏移
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// FABRIK 将左手移动到武器的插槽上
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// 获得插槽的 Transform
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);

		FVector OutVector;
		FRotator OutRotator;

		// 在骨骼空间中计算 hand_r 骨骼到 LeftHandTransform 的位置和旋转，保存到 OutVector 和 OutRotator 中
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutVector, OutRotator);
		
		LeftHandTransform.SetLocation(OutVector);
		LeftHandTransform.SetRotation(FQuat(OutRotator));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
		
	}

	TurnInPlace = BlasterCharacter->GetTurnInPlace();
	
}
