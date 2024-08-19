#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(Display = "SubmachineGun"),
	EWT_AssaultRiffle UMETA(Display = "AssaultRiffle"),
	EWT_Pistol UMETA(Display = "Pistol"),

	EWT_DefaultMax UMETA(Display = "DefaultMax")
};