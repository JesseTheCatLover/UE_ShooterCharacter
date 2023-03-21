// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	* Called via input to turn at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means %100 of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means %100 of desired turn rate
	*/
	void LookUpRate(float Rate);

	/**
	* Rotate controller based on mouse X movement
	* @param Value The input value from mouse movement
	*/
	void Turn(float Value);

	/**
	* Rotate controller based on mouse Y movement
	* @param Value The input value from mouse movement
	*/
	void LookUp(float Value);
	
	/**
	 * Called when the fire button is pressed
	 */
	void FireAction();

	/** Set bAiming to true or false with button pressed */
	void AimingButtonPressed();

	/** Set bAiming to true or false with button pressed */
	void AimingButtonReleased();

	/** Perform a line trace from crosshair screen location outward */
	bool LineTraceFromCrosshair(FHitResult &OutHitResult);

	/** Perform a second line trace from gun barrel to where the beam ends and mix the two trace together
	 *  and get the end location vector
	 *  @param MuzzleSocketLocation The location of gun barrel tip and where Muzzle particle spawns
	 *  @param OutBeamLocation Give out the location of where beam trace ends
	 */
	bool LineTraceFromGunBarrel(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/** Calculate camera interpolation zoom */
	void CameraInterpZoom(float DeltaTime);

	/** Set BaseTurnRate and BaseLookUpRate based on aiming state */
	void SetLookRates();

	/** Calculate dynamic crosshair animation */
	void CalculateCrosshairSpread(float DeltaTime);

	/** Crosshair BulletFire animation */
	UFUNCTION()
	void StartCrosshairBulletFire();
	UFUNCTION()
	void FinishCrosshairBulletFire();

	/** Automatic fire loop */
	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();
	UFUNCTION()
	void FireTimerReset();

	/** Trace for items if OverlappedItemCount > 0 */
	void PickupTrace();

	/** Spawn default Weapon for the character */
	class AWeapon* SpawnDefaultWeapon();

	/** Attach Weapon to HandSocket and apply appropriate properties */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/** Detach Weapon from HandSocket and drop it */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();
	
	void DropButtonPressed();
	void DropButtonReleased();

	/** Drops currently equipped Weapon and equips PickupTraceHitItem */
	void SwapWeapon(AWeapon* WeaponToSwap);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** Camera Boom Positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
	
	/** Camera that follows the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate in deg/sec. Other scaling may affect fina turn rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base turn rate in deg/sec. Other scaling may affect fina turn rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Scale factor for mouse look sensitivity, Turn rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse look sensitivity, Look up rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Scale factor for mouse look sensitivity, Turn rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;
	
	/** Scale factor for mouse look sensitivity, Look up rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	/** Flash spawned at BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;
	
	/** Montage for firing weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;
	
	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** True when aiming */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Default camera field of view value */
	float CameraDefaultFOV;
	
	/** Field of view value when the camera is zoomed in */
	float CameraZoomedFOV;

	/** Current camera field of view value */
	float CameraCurrentFOV;

	/** Interp speed for zooming when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Determines the spread of the crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadingMultiplier;

	/** Velocity component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/** Aim component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimingFactor;

	/** Shooting component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	/** Right mouse button or right console trigger pressed */
	bool bFireButtonPressed;

	/** True when we can fire. False when waiting for the timer */
	bool bShouldFire;
	
	/** True while gun is firing */
	bool bFiringBullet;

	/** Rate of automatic gun fire */
	float AutomaticFireRate;
	
	/** Duration of crosshair spread for shooting */
	float CrosshairShootingDuration;
	
	/** Sets a timer between gunshots */
	FTimerHandle AutomaticFireTimer;

	/** Sets a timer between crosshair spreads */
	FTimerHandle CrosshairShootTimer;

	/** True if we should trac every frame for items */
	bool bShouldTraceForItems;

	/** Number of overlapped AItem */
	int8 OverlappedItemCount;

	/** The AItem we currently hit by our trace in PickupTrace (could we null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class AItem* PickupTraceHitItem;
	
	/** The AItem we hit last frame in PickupTrace */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items , meta = (AllowPrivateAccess = "true"))
	AItem* PreviousPickupTraceHitItem;

	/** Currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat , meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	/** Set default weapon class blueprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat , meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/** Distance of desired location for Item pickup interpolation from the camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraPickupInterpDistance;

	/** Elevation of desired location for Item pickup interpolation from the camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraPickupInterpElevation;
	
public:
	/** Returns CameraBoom subObject */
	FORCEINLINE USpringArmComponent *GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subObject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }

	/** Returns CrosshairSpreadingMultiplier function */
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
	
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/** Adds/subtracts OverlappedItemCount and updates bShouldTraceForItems  */
	void IncrementOverlappedItemCount(int8 Value);

	/** Get the desired location for Item pick up interpolation */
	FVector GetPickupInterpTargetLocation();

	/** Set an item to pickup
	 * @param Item Item to pick up
	 */
	void PickupItem(AItem* Item);
};
