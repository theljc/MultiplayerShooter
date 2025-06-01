// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/CaptureTheFlag/FlagZone.h"

#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameMode/CTFGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"


AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
	
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag and OverlappingFlag->GetTeam() != Team)
	{
		ACTFGameMode* CTFGameMode = GetWorld()->GetAuthGameMode<ACTFGameMode>();
		 if (CTFGameMode)
		{
			CTFGameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
	
}


