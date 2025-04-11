// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


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

	void EquippedWeapon(AWeapon* WeaponToEquipped);

	void SetAiming(bool IsAiming);

	// 被标记为 Server 的函数，不管是从服务器还是从客户端调用，都只会在服务器上执行
	UFUNCTION(Server, Reliable)
	void Server_Aiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
protected:
	virtual void BeginPlay() override;
	

private:
	TObjectPtr<ABlasterCharacter> Character;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquipWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
	
};

