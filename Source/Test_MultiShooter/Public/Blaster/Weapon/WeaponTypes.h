#pragma once

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLaunch UMETA(DisplayName = "Rocket Launch"),
	
	EWT_DefaultMax UMETA(DisplayName = "Default Max")
};
