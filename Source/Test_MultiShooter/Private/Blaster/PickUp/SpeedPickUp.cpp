// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PickUp/SpeedPickUp.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"

void ASpeedPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (BlasterCharacter->GetBuffComponent())
		{
			BlasterCharacter->GetBuffComponent()->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}

	Destroy();
	
}
