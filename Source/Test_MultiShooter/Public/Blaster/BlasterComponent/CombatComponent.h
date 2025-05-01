// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABlasterHUD;
class ABlasterPlayerController;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEST_MULTISHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void PlayEquipWeaponSound();
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void ReloadEmptyWeapon();

	void EquippedWeapon(AWeapon* WeaponToEquipped);

	void SetAiming(bool IsAiming);

	// 被标记为 Server 的函数，不管是从服务器还是从客户端调用，都只会在服务器上执行
	UFUNCTION(Server, Reliable)
	void Server_Aiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	void FireButtonPressed(bool bPressed);

	void Reload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void Server_ThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	void UpdateAmmoValues();
	void JumpToShotGunEnd();
	void UpdateShotGunAmmoValues();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);

	// 多播 RPC 在服务器上执行时，会广播到服务器和所有客户端。在客户端执行时只会在自己的客户端执行，没有意义
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Fire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrossHair(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	bool CanFire();

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();
	
protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> CharacterPlayerController;
	UPROPERTY()
	ABlasterHUD* CharacterHUD;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquipWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	FTimerHandle FireTimer;

	bool bCanFire = true;
	
	void StartFireTimer();

	void FireTimerFinish();

	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	// TMap 无法被复制
	TMap<EWeaponTypes, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartRocketAmmo = 10;

	UPROPERTY(EditAnywhere)
	int32 StartPistolAmmo = 30;
	
	UPROPERTY(EditAnywhere)
	int32 StartSubmachineAmmo = 45;

	UPROPERTY(EditAnywhere)
	int32 StartShotGunAmmo = 20;

	UPROPERTY(EditAnywhere)
	int32 StartSniperAmmo = 10;

	UPROPERTY(EditAnywhere)
	int32 StartGrenadeAmmo = 15;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
};

