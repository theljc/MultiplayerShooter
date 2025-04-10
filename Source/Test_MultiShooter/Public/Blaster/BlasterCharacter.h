// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

UCLASS()
class TEST_MULTISHOOTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void LookUP(const FInputActionValue& Value);
	void EquippedButtonPressed();
	void CrouchButtonPressed();
	void AimingButtonPressed();
	void AimingButtonReleased();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> OverHeadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

	// RepNotify 通知时，会在服务器端修改变量的值，但不会触发 OnRep 函数
	// OnRep 函数可以有一个参数，当变量复制时，参数中会保存旧的变量值
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// RPC 函数
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UInputMappingContext> MappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_MoveRight;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Turn;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LookUP;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Equipped;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Aiming;
	
public:
	// 设置 OverlappingWeapon，由于 OverlappingWeapon 已经被注册为需要复制的变量，当 OverlappingWeapon 改变时，会根据条件复制到指定客户端
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();

	
};

