// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:
	void StopFalling();
	
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponDuration;
	/** True when Weapon is falling */
	bool bFalling;

	/** Amount of ammo in the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

public:
	/** Adds pulse to the Weapon */
	void ThrowWeapon();

	/** Called from Character class to decrement ammo value */
	void DecrementAmmo();
	
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
};
