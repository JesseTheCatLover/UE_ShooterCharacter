// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#include "ShooterCharacter.h"

#include "Ammo.h"
#include "Item.h"
#include "Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter():
	// Properties
	CurrentSpeed(0.f),
	// Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(41.f),
	// Turn rates for aiming/not aiming
	HipTurnRate(BaseTurnRate),
	HipLookUpRate(BaseLookUpRate),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse look sensitivity scale factors
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
	// Aiming
	bAiming(false),
	// Camera field of view values
	CameraDefaultFOV(0.f), // Set in BeginPlay
	CameraZoomedFOV(30.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(22.f),
	// Crosshair spread factors
	CrosshairSpreadingMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimingFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet fire variables
	bAimingButtonPressed(false),
	bFireButtonPressed(false),
	bShouldFire(true),
	bFiringBullet(false),
    AutomaticFireRate(0.1f),
	CrosshairShootingDuration(0.05f),
	// Item trace variables
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	// Starting ammo amounts
	Starting9mmAmmo(80),
	StartingARAmmo(120),
	// Combat state
	CombatState(ECombatState::ECS_Unoccupied),
	// Crouching
	bCrouching(false),
	bCrouchToggling(false),
	HipMovementSpeed(600.f),
	AimingMovementSpeed(400.f),
	CrouchMovementSpeed(280.f),
	HipCapsuleHalfHeight(88.f),
	CrouchCapsuleHalfHeight(46.f),
	CrouchWalkingCapsuleHalfHeight(65.f),
	CapsuleHalfHeightInterpSpeed(5.f),
	// Jumping
	bLandRecovering(false),
	JumpBoostVelocity(500.f),
	LandingRecoveryMovementSpeed(570.f),
	// Pickup/Equip sound timer
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundTimerDuration(0.2f),
	EquipSoundTimerDuration(0.2f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraBoom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom -> SetupAttachment(RootComponent);
	CameraBoom -> TargetArmLength = 220.f; // The camera follows at this distance behind the character
	CameraBoom -> bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom -> SocketOffset = FVector(0.f, 45.f, 70.f);

	// Create a FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera -> SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera -> bUsePawnControlRotation = false;

	// Don't rotate when the controller rotates. Let the controller only affect camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = true;
	bUseControllerRotationYaw = false;

	// Configure character movement
	GetCharacterMovement() -> bOrientRotationToMovement = false; // Character moves at the direction of input...
	GetCharacterMovement() -> RotationRate = FRotator(0.f, 540.f, 0.f); //... at this rate
	GetCharacterMovement() -> GravityScale = 1.4f;
	GetCharacterMovement() -> JumpZVelocity = 650.f;
	GetCharacterMovement() -> AirControl = 0.1f;

	// Create a ClipSceneComponent
	ClipSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ClipSceneComponent"));
	ClipSceneComponent -> SetupAttachment(GetMesh());

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp -> SetupAttachment(GetFollowCamera());
	
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1 -> SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2 -> SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3 -> SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4 -> SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5 -> SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6 -> SetupAttachment(GetFollowCamera());
	
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera() -> FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	// Spawn the default Weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());
	// Initialize AmmoMap with starting values
	InitializeAmmoMap();
	// Initialize InterpLocations for item picking interping
	InitializeInterpLocations();
}

void AShooterCharacter::UpdateProperties()
{
	// Getting the lateral Speed of the character from velocity
	FVector Velocity{ GetVelocity() }; Velocity.Z = 0.f;
	CurrentSpeed = Velocity.Size();
}

void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller -> GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation{ Controller -> GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld() -> GetDeltaSeconds()); // deg/sec * sec/frame
}


void AShooterCharacter::LookUpRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld() -> GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(float Value)
{
	const float TurnScaleFactor = bAiming ? MouseAimingTurnRate : MouseHipTurnRate;
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	const float LookUpScaleFactor = bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate;
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if(EquippedWeapon == nullptr) return; // if we are holding a weapon
	if(CombatState != ECombatState::ECS_Unoccupied) return; // if the weapon is available for firing

	if(WeaponHasAmmo())
	{
		// Visuals
		PlayFireSound();
		SendBullet();
		PlayHipFireMontage();
		
		// Decrement ammo
		EquippedWeapon -> DecrementAmmo();

		// Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();
		
		// Start FireRateTimer to kill off weapon, in order to simulate its fire rate
		StartFireRateTimer();
	}
}

bool AShooterCharacter::LineTraceFromCrosshair(FHitResult &OutHitResult) const
{
	// Get current size of the viewport
	FVector2D ViewportSize;
	if(GEngine && GEngine -> GameViewport)
	{
		GEngine -> GameViewport -> GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshair
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
	(UGameplayStatics::GetPlayerController(this, 0),
	 CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if(!bScreenToWorld) return false; // Was deprojection successful?

	const FVector Start{ CrosshairWorldPosition };
	const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

	// Trace outward from crosshairs world location
	GetWorld() -> LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
	
	if(OutHitResult.bBlockingHit) return true;
	
	OutHitResult.Location = End; // Mutating location
	return false;
}

bool AShooterCharacter::LineTraceFromGunBarrel(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) const
{
	FHitResult CrosshairHitResult;
	LineTraceFromCrosshair(CrosshairHitResult);
	OutBeamLocation = CrosshairHitResult.Location;
	
	/* If Crosshair's lineTrace hit something, OutBeamLocation will be set to
	 * the location of the hit,
	 * If it don't hit anything OutBeamLocation will be set to the point where lineTrace ends.
	 * However we still need another trace from the gun barrel:
	 */

	// Performs a second line trace, this time from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25 };

	GetWorld() -> LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd,
	                                     ECollisionChannel::ECC_Visibility);
	if(WeaponTraceHit.bBlockingHit) // Is there something between the barrel and the BeamEnd?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;	
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if(EquippedWeapon && CombatState != ECombatState::ECS_Reloading) // We can only aim when holding a Weapon
	{
		bAiming = true;
		if(!bCrouching) GetCharacterMovement() -> MaxWalkSpeed = AimingMovementSpeed;
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	bAiming = false;
	if(!bCrouching) GetCharacterMovement() -> MaxWalkSpeed = HipMovementSpeed;
}

void AShooterCharacter::SelectButtonPressed()
{
	if(PickupTraceHitItem)
	{
		PickupTraceHitItem -> StartPickingItem(this);

		PickupTraceHitItem = nullptr;
		PreviousPickupTraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::DropButtonPressed()
{
	DropWeapon();
}

void AShooterCharacter::DropButtonReleased()
{
	
}

void AShooterCharacter::HandleCameraInterpZoom(float DeltaTime)
{
	// Set current camera field of view
	if(bAiming)
	{
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera() -> SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const FVector2D WalkingSpeedRange { 0.f , 600.f };
	const FVector2D VelocityMultiplierRange { 0.f, 1.f };
	FVector Velocity { GetVelocity() };
	Velocity.Z = 0;

	if(GetCharacterMovement() -> IsFalling())
	{
		// Spread the crosshair slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);	
	}
	else
	{
		// Shrink the crosshair rapidly while on ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 35.f);	
	}

	// Calculate crosshair aim factor
	if(bAiming)
	{
		// Shrink the crosshair while aiming
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.5f, DeltaTime, 30.f);
	}
	else
	{
		// Spread the crosshair while not aiming
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.f, DeltaTime, 30.f);
	}

	// Calculate crosshair shoot factor
	if(bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.f);
	}
	
	// Calculate crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkingSpeedRange, VelocityMultiplierRange, Velocity.Size());
	CrosshairSpreadingMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimingFactor
	+ CrosshairShootingFactor;	
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire,
		CrosshairShootingDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::StartFireRateTimer()
{
	CombatState = ECombatState::ECS_FireRateTimerInProgress;
	GetWorldTimerManager().SetTimer(AutomaticFireRateTimer, this, &AShooterCharacter::FireRateTimerReset, AutomaticFireRate);
}

void AShooterCharacter::FireRateTimerReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else // Weapon is empty
	{
		ReloadWeapon();
	}
}

void AShooterCharacter::PickupTrace()
{
	if(bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		LineTraceFromCrosshair(ItemTraceResult);
		
		if(ItemTraceResult.bBlockingHit)
		{
			PickupTraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if(PickupTraceHitItem && PickupTraceHitItem -> GetPickupWidget())
			{
				// Show Item pickup widget 
				PickupTraceHitItem -> GetPickupWidget() -> SetVisibility(true);
			}

			// If linetrace hit an item last frame
			if(PreviousPickupTraceHitItem)
			{
				if(PickupTraceHitItem != PreviousPickupTraceHitItem) // If linetrace hit a new item this frame
				{
					PreviousPickupTraceHitItem -> GetPickupWidget() -> SetVisibility(false);
				}
			}
			
			// Saving a reference to the item linetrace hit, last frame. or either null ptr.
			PreviousPickupTraceHitItem = PickupTraceHitItem;
		}
	}
	else if(PreviousPickupTraceHitItem) // If character no longer overlap items, and the last item isn't null.
	{
		PreviousPickupTraceHitItem -> GetPickupWidget() -> SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	if(DefaultWeaponClass)
	{
		return GetWorld() -> SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh() -> GetSocketByName(FName("righthand_socket"));
		
		if(HandSocket)
		{
			HandSocket -> AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon -> SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(EquippedWeapon)
	{
		const FDetachmentTransformRules DetachmentTransformRule(EDetachmentRule::KeepWorld, true);
		EquippedWeapon -> GetItemMesh() -> DetachFromComponent(DetachmentTransformRule);
		
		EquippedWeapon -> SetItemState(EItemState::EIS_Falling);
		EquippedWeapon -> ThrowWeapon();
		EquippedWeapon = nullptr;
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);	
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);	
}

bool AShooterCharacter::WeaponHasAmmo() const
{
	if(EquippedWeapon == nullptr) return false; // If we aren't holding a weapon
	return EquippedWeapon -> GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound() const
{
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet() const
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon -> GetItemMesh() -> GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket -> GetSocketTransform(EquippedWeapon -> GetItemMesh());
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEndLocation;
		bool bBeamEnd = LineTraceFromGunBarrel(SocketTransform.GetLocation(), BeamEndLocation);
		if(bBeamEnd)
		{
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndLocation);
			}

			if(BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), BeamParticles, SocketTransform);
				if(Beam)
				{
					Beam -> SetVectorParameter(FName("Target"), BeamEndLocation);
				}
			}
		}
	}
}

void AShooterCharacter::PlayHipFireMontage() const
{
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance -> Montage_Play(HipFireMontage);
		AnimInstance -> Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(EquippedWeapon == nullptr) return;
	
	if(CarryingAmmo() && !EquippedWeapon -> ClipIsFull()) // are we carrying the correct type of ammo?
	{
		CombatState = ECombatState::ECS_Reloading;
		bAiming = false;
		if(!bCrouching) GetCharacterMovement() -> MaxWalkSpeed = HipMovementSpeed;
		UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			AnimInstance ->	Montage_Play(ReloadMontage);
			AnimInstance -> Montage_JumpToSection(EquippedWeapon -> GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingButtonPressed) bAiming = true;
	if(EquippedWeapon == nullptr) return;
	const auto AmmoType = EquippedWeapon -> GetAmmoType();
	
	if(AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon -> GetMagazineCapacity() - EquippedWeapon -> GetAmmo();

		if(CarriedAmmo < MagEmptySpace)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon -> ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		}
		else
		{
			// Fully fill the magazine
			EquippedWeapon -> ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
		}
		// Assign the value to AmmoMap
		AmmoMap.Add(AmmoType, CarriedAmmo);
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if(EquippedWeapon == nullptr) return false;

	const auto AmmoType = EquippedWeapon -> GetAmmoType();
	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

void AShooterCharacter::GrabClip()
{
	if(EquippedWeapon == nullptr) return;
	if(ClipSceneComponent == nullptr) return;
	
	const int32 ClipBoneIndex{ EquippedWeapon -> GetItemMesh() -> GetBoneIndex(EquippedWeapon -> GetClipBoneName()) };
	// Initial transform of the clip when hand first touches the clip
	ClipTransform = EquippedWeapon -> GetItemMesh() -> GetBoneTransform(ClipBoneIndex);
	
	const FAttachmentTransformRules AttachmentRules{ EAttachmentRule::KeepRelative, true };
	ClipSceneComponent -> AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	ClipSceneComponent -> SetWorldTransform(ClipTransform);

	EquippedWeapon -> SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip() const
{
	if(EquippedWeapon == nullptr) return;

	EquippedWeapon -> SetMovingClip(false);
}

void AShooterCharacter::CrouchingButtonPressed()
{
	if(!GetCharacterMovement() -> IsFalling() && !bCrouchToggling && !bLandRecovering)
	{
		bCrouching = !bCrouching; // Toggle
	}
	GetCharacterMovement() -> MaxWalkSpeed = bCrouching ? CrouchMovementSpeed : HipMovementSpeed;
}

void AShooterCharacter::StartCrouchToggle()
{
	bCrouchToggling = true;
	GetCharacterMovement() -> MaxWalkSpeed = 0.f;	
}

void AShooterCharacter::FinishCrouchToggle()
{
	bCrouchToggling = false;
	GetCharacterMovement() -> MaxWalkSpeed = bCrouching ? CrouchMovementSpeed : HipMovementSpeed;
}

void AShooterCharacter::HandleHalfHeightInterp(float DeltaTime) const
{
	const float Target{ bCrouching && CurrentSpeed > 0.f ? CrouchWalkingCapsuleHalfHeight : // not moving
	bCrouching ? CrouchCapsuleHalfHeight : HipCapsuleHalfHeight };
	const float CurrentHalfHeight = GetCapsuleComponent() -> GetScaledCapsuleHalfHeight();
	const float InterpValue = FMath::FInterpTo(CurrentHalfHeight, Target, DeltaTime, CapsuleHalfHeightInterpSpeed);

	// Positive value while standing, negative value while crouching.
	const float DeltaHalfHeight{ InterpValue - CurrentHalfHeight };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaHalfHeight };
	GetMesh() -> AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent() -> SetCapsuleHalfHeight(InterpValue);
}

void AShooterCharacter::Jump()
{
	if(!bCrouching && !bLandRecovering)
	{
		GetCharacterMovement() -> MaxWalkSpeed = HipMovementSpeed + JumpBoostVelocity;
		ACharacter::Jump();
	}
}

void AShooterCharacter::StartLanding()
{
	GetCharacterMovement() -> MaxWalkSpeed = LandingRecoveryMovementSpeed;
	bLandRecovering = true;
}

void AShooterCharacter::FinishLanding()
{
	GetCharacterMovement() -> MaxWalkSpeed = HipMovementSpeed;
	bLandRecovering = false;
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	const EAmmoType AmmoType{ Ammo -> GetAmmoType() };
	if(AmmoMap.Contains(AmmoType)) // if the ammo type is registered and valid
	{
		int32 AmmoCount{ AmmoMap[AmmoType] };
		AmmoCount += Ammo -> GetItemCount();
		AmmoMap[AmmoType] = AmmoCount;
		Ammo -> Destroy();
		
		if(EquippedWeapon == nullptr) return; // If we are holding a weapon
		// and it is empty and uses the same ammo type, after gathering ammo automatically reload it
		if(EquippedWeapon -> GetAmmoType() == AmmoType && EquippedWeapon -> GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
}

void AShooterCharacter::InitializeInterpLocations()
{
	const FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);

	const FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	const FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

	const FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);

	const FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);

	const FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);

	const FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::RestPickupSoundTimer()
{
	bShouldPlayPickupSound = true;	
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateProperties();
	
	// Interpolation for zoom when aiming
	HandleCameraInterpZoom(DeltaTime);
	// Change look sensitivity based on aiming state
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Trace for items while overlapping items
	PickupTrace();
	// Interpolation for CapsuleHalfHeight while standing/crouching
	HandleHalfHeightInterp(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn Right / Left", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("Look Up / Down", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AShooterCharacter::DropButtonPressed);
	PlayerInputComponent->BindAction("Drop", IE_Released, this, &AShooterCharacter::DropButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchingButtonPressed);
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadingMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Value)
{
	if(OverlappedItemCount + Value <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Value;
		bShouldTraceForItems = true;
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if(Index < InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	else
		return FInterpLocation();
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1, LowestCount = INT_MAX;

	for(int32 i = 1; i < InterpLocations.Num(); i++)
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Index < 0 || Amount < -1 || Amount > 1) return;
	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::PickupItem(AItem* Item)
{
	Item -> PlayEquipSound();
	
	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		SwapWeapon(Weapon);
		return;
	}

	auto Ammo = Cast<AAmmo>(Item);
	if(Ammo)
	{
		PickupAmmo(Ammo);
		return;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorld() -> GetTimerManager().SetTimer(PickupSoundTimer, this,
		&AShooterCharacter::ResetEquipSoundTimer, PickupSoundTimerDuration);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorld() -> GetTimerManager().SetTimer(EquipSoundTimer, this,
		&AShooterCharacter::ResetEquipSoundTimer, EquipSoundTimerDuration);
	
}