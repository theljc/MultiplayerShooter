// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterGameMode;
class UCharacterOverlay;
class ABlasterHUD;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnouncementCountDown(float CountDownTime);
	void SetHUDGrenades(int32 Grenades);
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 同步服务器的时间
	float GetServerTime();

	void OnMatchStateSet(FName State);
	void HandleMatchStart();
	void HandleCooldown();

	void HighPingWarning();
	void StopHighPingWarning();

	float SingleTripTime = 0.f;

protected:
	virtual void BeginPlay() override;
	void CheckTimeSync(float DeltaSeconds);
	void CheckPing(float DeltaSeconds);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDTime();
	void PollInit();

	// 向服务器请求当前服务器上的时间
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	// 客户端同步服务器的时间
	UFUNCTION(Client, Reliable)
	void Client_ReportServerTime(float TimeOfClientRequest, float TimeServerReceived);

	// 客户端和服务器之间的时间延迟
	float ClientServerDelta = 0.f;

	// 客户端同步服务器时间的频率
	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.f;

	// 距离上一次同步服务器时间过了多少时间
	float TimeSyncRunningTime = 0.f;

	UFUNCTION(Server, Reliable)
	void Server_CheckMatchState();

	UFUNCTION(Client, Reliable)
	void Client_JoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;

	UPROPERTY()
	TObjectPtr<UCharacterOverlay> CharacterOverlay;

	UPROPERTY()
	TObjectPtr<ABlasterGameMode> BlasterGameMode;

	// 判断是否已经初始化
	// bool bInitializeCharacterOverlay = false;

	// 保存的数据，在 HUD 有效时赋值
	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	
	float MatchTime = 0.f;
	float WarmUpTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;

	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	
};
