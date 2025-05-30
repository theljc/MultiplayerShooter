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

	void RedTeamScores();
	void BlueTeamScores();
	
	TArray<ABlasterPlayerState*> RedTeam;
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:
	float TopScore = 0.f;
	
};
