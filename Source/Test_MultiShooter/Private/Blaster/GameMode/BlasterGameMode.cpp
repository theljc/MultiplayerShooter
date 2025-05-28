// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/GameMode/BlasterGameMode.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName CooldownTime = FName("CooldownTime");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
	
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		// 计算关卡开始后经过的时间
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			// 调用后生成 Pawn 玩家
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::CooldownTime);
		}
	}
	else if (MatchState == MatchState::CooldownTime)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
	
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedPlayer, ABlasterPlayerController* VictimController,
                                        ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	
	if (AttackPlayerState and AttackPlayerState != VictimPlayerState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		
		AttackPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
		
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (ElimmedPlayer)
	{
		ElimmedPlayer->Elim(false);
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

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		// 如果退出的玩家在 TopScoringPlayers 中，则从 TopScoringPlayers 中删除
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		// 退出的玩家和死亡玩家统一处理
		CharacterLeaving->Elim(true);
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayerController)
		{
			BlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
	
}
