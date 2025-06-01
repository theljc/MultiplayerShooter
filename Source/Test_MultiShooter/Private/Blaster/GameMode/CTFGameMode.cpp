// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/GameMode/CTFGameMode.h"

#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Weapon/Flag.h"

void ACTFGameMode::PlayerEliminated(ABlasterCharacter* ElimmedPlayer, ABlasterPlayerController* VictimController,
                                    ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedPlayer, VictimController, AttackerController);
	
}

void ACTFGameMode::FlagCaptured(class AFlag* Flag, class AFlagZone* Zone)
{
	if (Flag->GetTeam() != Zone->Team)
	{
		ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
		if (BlasterGameState)
		{
			if (Zone->Team == ETeam::ET_BlueTeam)
			{
				BlasterGameState->BlueTeamScores();
			}

			if (Zone->Team == ETeam::ET_RedTeam)
			{
				BlasterGameState->RedTeamScores();
			}
		}
	}
}
