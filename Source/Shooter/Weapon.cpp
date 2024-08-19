// Copyright (c) 2024 JesseTheCatLover. All Rights Reserved.


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
	ClipBoneName(FName(TEXT("smg_clip"))),
	bShouldHideBone(false),
	BoneToHide(TEXT("")),
	SlideDisplacement(0.f),
	SlideDuration(0.2f),
	MaxSlideDisplacement(4.f),
	bMovingSlide(false),
	RecoilRotation(0.f),
	MaxRecoilRotation(20.f),
	bAutomatic(true)
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
	// Update slide on pistol
	UpdateSlideDisplacement();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	LoadWeaponTypeData();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	LoadWeaponTypeData();
	if(bShouldHideBone && BoneToHide != TEXT(""))
	{
		GetItemMesh() -> HideBoneByName(BoneToHide, PBO_None);
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

void AWeapon::LoadWeaponTypeData()
{
	
	const FString WeaponTablePath{TEXT("DataTable'/Game/_Game/DataTables/WeaponDataTable.WeaponDataTable'")};
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if(WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;
		switch(WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject -> FindRow<FWeaponDataTable>(TEXT("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssaultRiffle:
			WeaponDataRow = WeaponTableObject -> FindRow<FWeaponDataTable>(TEXT("AssaultRiffle"), TEXT(""));
			break;
		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject -> FindRow<FWeaponDataTable>(TEXT("Pistol"), TEXT(""));
		}

		if(WeaponDataRow)
		{
			SetItemName(WeaponDataRow -> WeaponName);
			GetItemMesh() -> SetSkeletalMesh(WeaponDataRow -> WeaponMesh);
			SetItemIcon(WeaponDataRow -> WeaponIcon);
			SetAmmoTypeIcon(WeaponDataRow -> AmmoTypeIcon);
			AmmoType = WeaponDataRow -> AmmoType;
			Ammo = WeaponDataRow -> StartingAmmo;
			MagazineCapacity = WeaponDataRow -> MagazineCapacity;
			SetPickupSound(WeaponDataRow -> PickupSound);
			SetEquipSound(WeaponDataRow -> EquipSound);
			SetGlowMaterialIndex(WeaponDataRow -> GlowMaterialIndex);
			ClipBoneName = WeaponDataRow -> ClipBoneName;
			ReloadMontageSection = WeaponDataRow -> ReloadMontageSectionName;
			GetItemMesh() -> SetAnimInstanceClass(WeaponDataRow -> AnimBP);
			CrosshairMiddle = WeaponDataRow -> CrosshairMiddle;
			CrosshairRight = WeaponDataRow -> CrosshairRight;
			CrosshairLeft = WeaponDataRow -> CrosshairLeft;
			CrosshairTop = WeaponDataRow -> CrosshairTop;
			CrosshairBottom = WeaponDataRow -> CrosshairBottom;
			AutoFireRate = WeaponDataRow -> AutoFireRate;
			MuzzleFlash = WeaponDataRow -> MuzzleFlash;
			FireSound = WeaponDataRow -> FireSound;
			bShouldHideBone = WeaponDataRow -> bShouldHideBone;
			BoneToHide = WeaponDataRow -> BoneToHide;
			MaxSlideDisplacement = WeaponDataRow -> MaxSlideDisplacement;
			MaxRecoilRotation = WeaponDataRow -> MaxRecoilRotation;
			SlideDuration = WeaponDataRow -> SlideDuration;
			SlideDisplacementCurve = WeaponDataRow -> SlideDisplacementCurve;
			bAutomatic = WeaponDataRow -> bAutomatic;

			GetItemMesh() -> SetMaterial(GetPreviousMaterialIndex(), nullptr);
			SetMaterialInstance(WeaponDataRow -> MaterialInstance);
			if(GetMaterialInstance())
			{
				SetGlowMaterialInstanceDynamic(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
				GetGlowMaterialInstanceDynamic() -> SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
				GetItemMesh() -> SetMaterial(GetGlowMaterialIndex(), GetGlowMaterialInstanceDynamic());
				EnableGlowMaterial();
			}
		}
	}
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this,  &AWeapon::SlideTimerFinished, SlideDuration);
}

void AWeapon::SlideTimerFinished()
{
	bMovingSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
	if(SlideDisplacementCurve && bMovingSlide)
	{
		const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(SlideTimer) };
		const float CurveValue{ SlideDisplacementCurve -> GetFloatValue(ElapsedTime) };
		SlideDisplacement = CurveValue * MaxSlideDisplacement;
		RecoilRotation = CurveValue * MaxRecoilRotation;
	}
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
