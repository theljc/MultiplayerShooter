// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

namespace MatchState
{
	extern TEST_MULTISHOOTER_API const FName CooldownTime;
}

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();
	
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedPlayer, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedPlayer, AController* ElimmedController);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountDownTime = 0.f;
	
};
