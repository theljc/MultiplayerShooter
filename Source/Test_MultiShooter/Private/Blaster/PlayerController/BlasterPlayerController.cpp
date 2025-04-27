// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/PlayerController/BlasterPlayerController.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
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

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();
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
	
	int32 SecondLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
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
				if (bInitializeCharacterOverlay)
				{
					SetHUDHealth(HUDHealth, HUDMaxHealth);
					SetHUDScore(HUDScore);
					SetHUDDefeats(HUDDefeats);
				}
			}
		}
	}
	
}

void ABlasterPlayerController::Server_CheckMatchState_Implementation()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
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
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
		bInitializeCharacterOverlay = true;
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
		bInitializeCharacterOverlay = true;
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
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? TObjectPtr<ABlasterHUD>(Cast<ABlasterHUD>(GetHUD())) : BlasterHUD;
	if (BlasterHUD and BlasterHUD->CharacterOverlay)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
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
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60.f;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmUpTime->SetText(FText::FromString(CountDownText));
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
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	// 乘 0.5 来近似得到当前服务器的时间
	float CurrentServerTime = TimeServerReceived + (0.5 * RoundTripTime);
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
		}
	}
}
