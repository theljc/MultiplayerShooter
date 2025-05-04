// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PickUp/ShieldPickUp.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"

void AShieldPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (BlasterCharacter->GetBuffComponent())
		{
			BlasterCharacter->GetBuffComponent()->BuffShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}

	Destroy();
	
}
