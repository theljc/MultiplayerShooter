// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/GameMode/BlasterGameMode.h"

#include "Blaster/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedPlayer, ABlasterPlayerController* VictimController,
                                        ABlasterPlayerController* AttackerController)
{
	if (ElimmedPlayer)
	{
		ElimmedPlayer->MulticastEliminate();
	}
	
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedPlayer, AController* ElimmedController)
{
	if (ElimmedPlayer)
	{
		ElimmedPlayer->Reset();
		ElimmedPlayer->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
	
}
