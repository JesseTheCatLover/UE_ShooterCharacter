// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "Weapon.h"

AWeapon::AWeapon():
	ThrowWeaponDuration(0.7f),
	bFalling(false),
	Ammo(30),
	MagazineCapacity(30),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("RELOAD_SMG"))),
	bMovingClip(false),
	ClipBoneName(FName(TEXT("smg_clip")))
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep the Weapon upright
	if(GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		FRotator MeshRotation{ 0.f, GetItemMesh() -> GetComponentRotation().Yaw, 0.f };
		GetItemMesh() -> SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	// Keeping the Weapon still on its yaw rotation
	FRotator MeshRotation { 0.f, GetItemMesh() -> GetComponentRotation().Yaw, 0.f };
	GetItemMesh() -> SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	
	const FVector MeshForward { GetItemMesh() -> GetForwardVector() };
	const FVector MeshRight { GetItemMesh() -> GetRightVector() };
	// Direction in which we throw the Weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-50.f, MeshForward);

	const float RandomRotation { 30.f };

	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 10'000.f;
	
	GetItemMesh() -> AddImpulse(ImpulseDirection);
	bFalling = true;

	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponDuration);
	
	EnableGlowMaterial();
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	StartGlowPulseTimer();
}

void AWeapon::DecrementAmmo()
{
	if(Ammo - 1 <= 0) Ammo = 0;
	else --Ammo;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to overfill the magazine"));
	Ammo += Amount;
}
