// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USoundCue;
class ABlasterPlayerController;
class ABlasterCharacter;
class ACasing;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWC_Initial UMETA(DisplayName = "Weapon Initial"),
	EWC_Equipped UMETA(DisplayName = "Equipped"),
	EWC_EquipSecondary UMETA(DisplayName = "Equip Secondary"),
	EWC_Dropped UMETA(DisplayName = "Dropped"),
	EWC_MAX UMETA(DisplayName = "DefaultMAX"),
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	
	EFT_MAX UMETA(DisplayName = "DefaultMAX"),
	
};

UCLASS()
class TEST_MULTISHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	void ShowPickUpWidget(bool bShow);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoAmount);

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	TObjectPtr<UTexture2D> CrossHairs;

	UPROPERTY(EditAnywhere, Category=Combat)
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category=Combat)
	bool bAutoFire = true;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> EquippedSound;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:
	UPROPERTY(EditAnywhere)
	float Damage = 10.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float DistanceToSphere = 800.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float SphereRadius = 75.f;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

private:
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	TObjectPtr<UWidgetComponent> PickUpWidget;

	UPROPERTY(EditAnywhere, Category="Weapon")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<ACasing> CasingClass;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaponState = EWeaponState::EWC_Initial;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedInterpSpeed = 20.f;

	// 此属性在客户端预测
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	void SpendRound();

	// 子弹容量
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// 用于客户端预测 Ammo 属性时，标记自上一次服务器同步以来有多少子弹被消耗了
	int32 Sequence = 0;

	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterOwnerCharacter;

	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> BlasterOwnerPlayerController;
	
	// 武器状态改变时复制
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere)
	EWeaponTypes WeaponTypes;

public:
	void SetWeaponState(EWeaponState NewWeaponState);
	void OnWeaponStateSet();
	void EquipWeapon();
	void DropWeapon();
	void EquipSecondaryWeapon();
	
	FORCEINLINE USphereComponent* GetSphereComponent() const { return SphereComponent; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomedInterpSpeed; }
	FORCEINLINE EWeaponTypes GetWeaponType() const { return WeaponTypes; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	
	bool IsAmmoEmpty();
	bool IsAmmoFull();
	
};

