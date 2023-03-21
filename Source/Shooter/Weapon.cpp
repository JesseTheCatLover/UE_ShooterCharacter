// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "Weapon.h"

AWeapon::AWeapon():
	ThrowWeaponDuration(0.7f),
	bFalling(false)
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
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}
