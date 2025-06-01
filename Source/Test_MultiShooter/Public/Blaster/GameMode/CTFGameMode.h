// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ACTFGameMode : public ATeamGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedPlayer, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);

};
