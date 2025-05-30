// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerState;
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
	// 玩家退出游戏
	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	// 游戏一开始的热身时间
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	// 每局游戏的持续时间
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	// 每局重新开始的间隔时间
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// 保存关卡开始的时间
	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountDownTime = 0.f;

public:
	FORCEINLINE float GetCountDownTime() { return CountDownTime; }
	
};
