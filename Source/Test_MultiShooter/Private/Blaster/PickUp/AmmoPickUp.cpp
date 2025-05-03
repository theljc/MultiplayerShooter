// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PickUp/AmmoPickUp.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/CombatComponent.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (BlasterCharacter->GetCombatComponent())
		{
			BlasterCharacter->GetCombatComponent()->PickUpAmmo(WeaponTypes, AmmoAmount);
		}
	}

	Destroy();
	
}
