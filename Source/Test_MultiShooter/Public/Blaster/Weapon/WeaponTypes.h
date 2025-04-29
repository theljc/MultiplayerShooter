#pragma once

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLaunch UMETA(DisplayName = "Rocket Launch"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	
	EWT_DefaultMax UMETA(DisplayName = "Default Max")
};
