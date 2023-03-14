// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Item.h"
#include "Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter():
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
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(22.f),
	// Crosshair spread factors
	CrosshairSpreadingMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimingFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet fire variables
	bFireButtonPressed(false),
	bShouldFire(true),
	bFiringBullet(false),
    AutomaticFireRate(0.1f),
	CrosshairShootingDuration(0.05f),
	// Item trace variables
	bShouldTraceForItems(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraBoom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom -> SetupAttachment(RootComponent);
	CameraBoom -> TargetArmLength = 180.f; // The camera follows at this distance behind the character
	CameraBoom -> bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom -> SocketOffset = FVector{ 0.f, 50.f, 70.f };

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
	GetCharacterMovement() -> JumpZVelocity = 600.f;
	GetCharacterMovement() -> AirControl = 0.1f;
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
}

void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		FRotator Rotation{ Controller -> GetControlRotation() };
		FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		FRotator Rotation{ Controller -> GetControlRotation() };
		FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		FVector Direction{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::Y) };
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
	float TurnScaleFactor{};
	if(bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if(bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireAction()
{
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh() -> GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket -> GetSocketTransform(GetMesh());
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
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance -> Montage_Play(HipFireMontage);
		AnimInstance -> Montage_JumpToSection(FName("StartFire"));
	}

	// Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
}

bool AShooterCharacter::LineTraceFromCrosshair(FHitResult &OutHitResult)
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
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
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

bool AShooterCharacter::LineTraceFromGunBarrel(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
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
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;	
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::DropButtonPressed()
{
	DropWeapon();	
}

void AShooterCharacter::DropButtonReleased()
{
	
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
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
	FVector2D WalkingSpeedRange { 0.f , 600.f };
	FVector2D VelocityMultiplierRange { 0.f, 1.f };
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

void AShooterCharacter::StartFireTimer()
{
	if(bShouldFire)
	{
		bShouldFire = false;
		FireAction();
		GetWorldTimerManager().SetTimer(AutomaticFireTimer, this, &AShooterCharacter::FireTimerReset, AutomaticFireRate);
	}
}

void AShooterCharacter::FireTimerReset()
{
	bShouldFire = true;
	if(bFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::TraceForItems()
{
	if(bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		LineTraceFromCrosshair(ItemTraceResult);
		
		if(ItemTraceResult.bBlockingHit)
		{
			AItem* TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if(TraceHitItem && TraceHitItem -> GetPickupWidget())
			{
				// Show Item pickup widget
				TraceHitItem -> GetPickupWidget() -> SetVisibility(true);
			}

			// if linetrace hit an item last frame
			if(PreviousTraceHitItem)
			{
				if(TraceHitItem != PreviousTraceHitItem) // if linetrace hit a new item this frame
				{
					PreviousTraceHitItem -> GetPickupWidget() -> SetVisibility(false);
				}
			}
			
			// Saving a reference to the item linetrace hit, last frame. or either null ptr.
			PreviousTraceHitItem = TraceHitItem;
		}
	}
	else if(PreviousTraceHitItem) // if character no longer overlap items, and the last item isn't null.
	{
		PreviousTraceHitItem -> GetPickupWidget() -> SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
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
		FDetachmentTransformRules DetachmentTransformRule(EDetachmentRule::KeepWorld, true);
		EquippedWeapon -> GetItemMesh() -> DetachFromComponent(DetachmentTransformRule);

		EquippedWeapon -> SetItemState(EItemState::EIS_Falling);
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);
	// Change look sensitivity based on aiming state
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Trace for items while overlapping items
	TraceForItems();
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AShooterCharacter::DropButtonPressed);
	PlayerInputComponent->BindAction("Drop", IE_Released, this, &AShooterCharacter::DropButtonReleased);
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
