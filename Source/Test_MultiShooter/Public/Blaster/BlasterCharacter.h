// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurnInPlace.h"
#include "Components/TimelineComponent.h"
#include "Interface/InteractCrosshair_Interface.h"
#include "BlasterCharacter.generated.h"

class ABlasterPlayerController;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

UCLASS()
class TEST_MULTISHOOTER_API ABlasterCharacter : public ACharacter, public IInteractCrosshair_Interface
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

	virtual void OnRep_ReplicatedMovement() override;
	void UpdateHUDHealth();

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Jump() override;

	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void LookUP(const FInputActionValue& Value);
	void EquippedButtonPressed();
	void CrouchButtonPressed();
	void AimingButtonPressed();
	void AimingButtonReleased();
	void CalcAO_Pitch();
	void AimingOffset(float DeltaTime);
	void SetTurnInPlace(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();

	void HideCameraIfCharacterClosed();

	void SimProxiesTurn();

	
	
	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;

	FRotator StartAimRotation;


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> OverHeadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> FireMontage_Aim_Hip;

	ETurnInPlace TurnInPlace;

	// RepNotify 通知时，会在服务器端修改变量的值，但不会触发 OnRep 函数
	// OnRep 函数可以有一个参数，当变量复制时，参数中会保存旧的变量值
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// RPC 函数
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> ElimMontage;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalcSpeed();

	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	TObjectPtr<ABlasterPlayerController> BlasterPlayerController;

	bool bElimed = false;

	FTimerHandle ElimTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	
	void ElimTimeFinished();

	
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTimelineComponent> DissolveTimeline;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> DissolveCurve;

	// 蓝图设置后创建的动态材质实例
	UPROPERTY(VisibleAnywhere, Category=Elim)
	TObjectPtr<UMaterialInstanceDynamic> DynamicDissolveMaterialInstance;

	// 蓝图中设置的材质实例
	UPROPERTY(EditAnywhere, Category=Elim)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Fire;
	
public:
	// 设置 OverlappingWeapon，由于 OverlappingWeapon 已经被注册为需要复制的变量，当 OverlappingWeapon 改变时，会根据条件复制到指定客户端
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();
	
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);


	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurnInPlace GetTurnInPlace() { return TurnInPlace; }
	FORCEINLINE UCameraComponent* GetCamera() { return Camera; }
	FORCEINLINE bool ShouldRotateRootBone() { return bRotateRootBone; }
	FORCEINLINE bool IsElimed() { return bElimed; }

	TObjectPtr<AWeapon> GetWeapon();

	FVector GetHitTarget();
	
};

