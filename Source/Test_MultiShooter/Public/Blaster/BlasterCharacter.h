// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurnInPlace.h"
#include "BlasterTypes/CombatState.h"
#include "Components/TimelineComponent.h"
#include "Interface/InteractCrosshair_Interface.h"
#include "BlasterCharacter.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ULagCompensationComponent;
class UBoxComponent;
class UBuffComponent;
class ABlasterGameMode;
class ABlasterPlayerState;
class USoundCue;
class ABlasterPlayerController;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void DropOrDestroyWeapon(AWeapon* Weapon);

	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScope(bool bShowScope);

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, TObjectPtr<UBoxComponent>> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Jump() override;
	virtual void Destroyed() override;

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
	void ReloadButtonPressed();
	void ThrowGrenadeButtonPressed();

	void HideCameraIfCharacterClosed();

	void SimProxiesTurn();

	void PollInit();

	void RotateInPlace(float DeltaTime);
	
	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;

	FRotator StartAimRotation;

	/**
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> head;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> pelvis;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> spine_02;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> spine_03;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> upperarm_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> upperarm_r;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> lowerarm_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> lowerarm_r;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> hand_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> hand_r;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> backpack;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> blanket;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> thigh_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> thigh_r;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> calf_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> calf_r;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> foot_l;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> foot_r;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBuffComponent> BuffComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULagCompensationComponent> LagCompensationComponent;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> ElimMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> GrenadeMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> SwapMontage;
	
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
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Shield, VisibleAnywhere, Category="Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> BlasterPlayerController;

	bool bElimed = false;

	FTimerHandle ElimTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	
	void ElimTimeFinished();

	bool bLeftGame = false;

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


	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> CrownSystem;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> CrownComponent;
	

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> ElimBotEffect;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystemComponent> ElimBotEffectComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ElimBotSound;

	UPROPERTY()
	TObjectPtr<ABlasterPlayerState> BlasterPlayerState;

	UPROPERTY()
	TObjectPtr<ABlasterGameMode> BlasterGameMode;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AttachGrenade;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Reload;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ThrowGrenade;
	
public:
	// 设置 OverlappingWeapon，由于 OverlappingWeapon 已经被注册为需要复制的变量，当 OverlappingWeapon 改变时，会根据条件复制到指定客户端
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();
	
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);


	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurnInPlace GetTurnInPlace() { return TurnInPlace; }
	FORCEINLINE UCameraComponent* GetCamera() { return Camera; }
	FORCEINLINE bool ShouldRotateRootBone() { return bRotateRootBone; }
	FORCEINLINE bool IsElimed() { return bElimed; }
	FORCEINLINE float GetHealth() { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetMaxHealth() { return MaxHealth; }
	FORCEINLINE float GetShield() { return Shield; }
	FORCEINLINE void SetShield(float NewShield) { Shield = NewShield; }
	FORCEINLINE float GetMaxShield() { return MaxShield; }
	FORCEINLINE UCombatComponent* GetCombatComponent() { return CombatComponent; }
	FORCEINLINE bool GetDisplayGameplay() { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() { return AttachGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent() { return BuffComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() { return LagCompensationComponent; }

	bool IsLocallyReloading();
	
	ECombatState GetCombatState() const;
	
	TObjectPtr<AWeapon> GetWeapon();

	FVector GetHitTarget();
	
};

