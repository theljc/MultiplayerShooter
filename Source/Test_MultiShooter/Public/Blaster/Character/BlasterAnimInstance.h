// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurnInPlace.h"
#include "BlasterAnimInstance.generated.h"

class AWeapon;
class ABlasterCharacter;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
private:
	UPROPERTY(BlueprintReadWrite, Category = Character, meta = (AllowPrivateAccess = true))
	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	float Speed;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAcceleration;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;

	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAiming;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	float YawOffset;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	float Lean;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	float AO_Yaw;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	float AO_Pitch;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	ETurnInPlace TurnInPlace;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	FRotator RightHandRotation;
	
	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsLocallyControlled = false;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bElimed;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseAimOffset;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bTransformRightHand;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bHoldingFlag;
	
	FRotator LastFrameCharacterRotator;
	FRotator CharacterRotator;
	FRotator DeltaRotation;
	
};
