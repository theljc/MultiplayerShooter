// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 得分最高的玩家可能有多个
	UPROPERTY(Replicated)
	TArray<TObjectPtr<ABlasterPlayerState>> TopScoringPlayers;

	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

private:
	float TopScore = 0.f;
	
};
