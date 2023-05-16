#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assualt Riffle"),

	EAT_Max UMETA(DisplayName = "DefaultMax")
};