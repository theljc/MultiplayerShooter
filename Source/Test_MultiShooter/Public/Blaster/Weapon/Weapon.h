// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACasing;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWC_Initial UMETA(DisplayName = "Weapon Initial"),
	EWC_Equipped UMETA(DisplayName = "Equipped"),
	EWC_Dropped UMETA(DisplayName = "Dropped"),
	EWC_MAX UMETA(DisplayName = "DefaultMAX"),
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
	virtual void Fire(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	UTexture2D* CrossHairs;
	
protected:
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

	// 武器状态改变时复制
	UFUNCTION()
	void OnRep_WeaponState();
	
public:
	void SetWeaponState(EWeaponState NewWeaponState);
	FORCEINLINE USphereComponent* GetSphereComponent() const { return SphereComponent; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh; }
	
};
