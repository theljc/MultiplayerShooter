#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLaunch UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_ShotGun UMETA(DisplayName = "ShotGun"),
	EWT_Sniper UMETA(DisplayName = "Sniper"),
	EWT_GrenadeLaunch UMETA(DisplayName = "Grenade Launcher"),
	EWT_Flag UMETA(DisplayName = "Flag"),

	EWT_DefaultMax UMETA(DisplayName = "Default Max")
};
