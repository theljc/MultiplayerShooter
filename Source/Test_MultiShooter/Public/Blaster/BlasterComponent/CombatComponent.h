// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AProjectile;
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
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void ReloadEmptyWeapon();

	void EquippedWeapon(AWeapon* WeaponToEquipped);

	void SetAiming(bool IsAiming);

	// 被标记为 Server 的函数，不管是从服务器还是从客户端调用，都只会在服务器上执行
	UFUNCTION(Server, Reliable)
	void Server_Aiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryEquipWeapon();
	
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);


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

	// WithValidation 防止作弊
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	// 多播 RPC 在服务器上执行时，会广播到服务器和所有客户端。在客户端执行时只会在自己的客户端执行，没有意义
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	void TraceUnderCrossHair(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	bool CanFire();

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void Server_LaunchGrenade(const FVector_NetQuantize& Target);

	void PickUpAmmo(EWeaponTypes WeaponTypes, int32 AmmoAmount);

	void SwapWeapons();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	
	bool bLocallyReloading = false;

protected:
	virtual void BeginPlay() override;
	void ShowAttachGrenade(bool bShowGrenade);

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	void EquipPrimaryButton(AWeapon* WeaponToEquip);
	void EquipSecondaryButton(AWeapon* WeaponToEquip);
	
private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> CharacterPlayerController;
	UPROPERTY()
	ABlasterHUD* CharacterHUD;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquipWeapon;

	UPROPERTY(ReplicatedUsing=OnRep_SecondaryEquipWeapon)
	TObjectPtr<AWeapon> SecondaryEquipWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

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

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;
	
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

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();
	
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();

public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
	
};

