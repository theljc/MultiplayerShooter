// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PlayerController/BlasterPlayerController.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	
	Server_CheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	
}

void ABlasterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	// 如果未同步时间大于指定时间则进行同步
	if (IsLocalController() and TimeSyncRunningTime > TimeSyncFrequency)
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? TObjectPtr<APlayerState>(GetPlayerState<APlayerState>()) : PlayerState;
		// 获得压缩后的 Ping 值，乘 4 还原原来的值
		if (PlayerState->GetCompressedPing() * 4.f > HighPingThreshold)
		{
			HighPingWarning();
			PingAnimationRunningTime = 0.f;
		}
		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimationPlaying =
		BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnim &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnim);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaSeconds;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();

	CheckPing(DeltaSeconds);
	
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
	
}

void ABlasterPlayerController::SetHUDTime()
{
	// 如果玩家是之后加入进来的，则计算当前经过的时间
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::CooldownTime)
	{
		TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	int32 SecondLeft = FMath::CeilToInt(TimeLeft);

	// 重新开始游戏时重新从服务器同步时间
	if (HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? TObjectPtr<ABlasterGameMode>(Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this))) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondLeft = FMath::CeilToInt(BlasterGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}
	
	if (CountdownInt != SecondLeft)
	{
		if (MatchState == MatchState::WaitingToStart or MatchState == MatchState::CooldownTime)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
		
	}

	CountdownInt = SecondLeft;
	
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD and BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter and BlasterCharacter->GetCombatComponent())
				{
					SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
				}
			}
		}
	}
	
}

void ABlasterPlayerController::Server_CheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? TObjectPtr<ABlasterGameMode>(Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this))) : BlasterGameMode;
	if (BlasterGameMode)
	{
		WarmUpTime = BlasterGameMode->WarmUpTime;
		MatchTime = BlasterGameMode->MatchTime;
		CooldownTime = BlasterGameMode->CooldownTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		MatchState = BlasterGameMode->GetMatchState();
		
		Client_JoinMidGame(MatchState, WarmUpTime, MatchTime, CooldownTime, LevelStartingTime);

	}
}

void ABlasterPlayerController::Client_JoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,
	float Cooldown, float StartingTime)
{
	WarmUpTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (BlasterHUD and MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
	
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;

	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
	
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;

	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldBarText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;

	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;

	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;

	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60.f;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD and BlasterHUD->Announcement)
	{
		if (CountDownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmUpTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60.f;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmUpTime->SetText(FText::FromString(CountDownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadeText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
	
}

void ABlasterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTime = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeOfClientRequest, ServerTime);
}

void ABlasterPlayerController::Client_ReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceived)
{
	// 从客户端到服务器再到客户端的往返时间
	SingleTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	// 乘 0.5 来近似得到当前服务器的时间
	float CurrentServerTime = TimeServerReceived + (0.5 * SingleTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
	
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();

	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
	
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	// 只在服务器上执行，切换为 InProgress 状态
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
	else if (MatchState == MatchState::CooldownTime)
	{
		HandleCooldown();
	}
	
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
	else if (MatchState == MatchState::CooldownTime)
	{
		HandleCooldown();
	}
	
}

void ABlasterPlayerController::HandleMatchStart()
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr)
			BlasterHUD->AddCharacterOverlay();
		
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString Announcement = FString("New Match Start In");
			BlasterHUD->Announcement->AnnoucementText->SetText(FText::FromString(Announcement));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			
			if (BlasterGameState and BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> Players = BlasterGameState->TopScoringPlayers;
				FString InfoTextString;
				if (Players.Num() == 0)
				{
					InfoTextString = FString::Printf(TEXT("No Winners"));
				}
				else if (Players.Num() == 1 and Players[0] == BlasterPlayerState)
				{
					InfoTextString = FString::Printf(TEXT("You Are Winner"));
				}
				else if (Players.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: %s"), *Players[0]->GetPlayerName());
				}
				else if (Players.Num() > 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n"));
					for (auto TopPlayer : Players)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
					}
					
				}
				
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
			
		}
	}

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter and BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
	
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->Image_HighPing &&
		BlasterHUD->CharacterOverlay->HighPingAnim;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->Image_HighPing->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(
			BlasterHUD->CharacterOverlay->HighPingAnim,
			0.f,
			5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->Image_HighPing &&
		BlasterHUD->CharacterOverlay->HighPingAnim;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->Image_HighPing->SetOpacity(0.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnim))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnim);
		}
	}
}
