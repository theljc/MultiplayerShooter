// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	virtual void Dropped() override;

protected:
	virtual void EquipWeapon() override;
	virtual void DropWeapon() override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FlagMesh;
	
};
