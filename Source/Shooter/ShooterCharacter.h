// Copyright 2024 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireRateTimerInProgress UMETA(DisplayName = "FireRateTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	
	ECS_Max UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY();

	// Scene component to use for its location for interpolation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	// Number of items interping to/at this SceneComponent location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

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

	/** Update character properties */
	void UpdateProperties();
	
	/** Called for forward/backward input */
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
	void FireWeapon();

	/** Set bAiming to true or false with button pressed */
	void AimingButtonPressed();

	/** Set bAiming to true or false with button pressed */
	void AimingButtonReleased();

	void Aim();
	void StopAiming();

	/** Perform a line trace from crosshair screen location outward */
	bool LineTraceFromCrosshair(FHitResult &OutHitResult) const;

	/** Perform a second line trace from gun barrel to where the beam ends and mix the two trace together
	 *  and get the end location vector
	 *  @param MuzzleSocketLocation The location of gun barrel tip and where Muzzle particle spawns
	 *  @param OutBeamLocation Give out the location of where beam trace ends
	 */
	bool LineTraceFromGunBarrel(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) const;

	/** Calculate camera interpolation zoom */
	void HandleCameraInterpZoom(float DeltaTime);

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
	void StartFireRateTimer();
	UFUNCTION()
	void FireRateTimerReset();

	/** Trace for items if OverlappedItemCount > 0 */
	void PickupTrace();

	/** Spawn default Weapon for the character */
	class AWeapon* SpawnDefaultWeapon() const;

	/** Attach Weapon to HandSocket and apply appropriate properties */
	void EquipWeapon(AWeapon* WeaponToEquip,  bool bSwapping = false);

	/** Detach Weapon from HandSocket and drop it */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();
	
	void DropButtonPressed();
	void DropButtonReleased();

	/** Drops currently equipped Weapon and equips PickupTraceHitItem */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/** Initialize the AmmoMap with ammo values */
	void InitializeAmmoMap();

	/** Return true if EquippedWeapon has ammo */
	bool WeaponHasAmmo() const;

	/** Play firing sound */
	void PlayFireSound() const;

	/** Perform linetrace for shooting and gathering information */
	void SendBullet() const;

	/** Play HipFire montage animation */
	void PlayHipFireMontage() const;
	
	void ReloadButtonPressed();

	/** Handle reloading the weapon */
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/** Return true if the character is carrying the type of ammo matching to EquippedWeapon's */
	bool CarryingAmmo();
	
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip() const;

	void CrouchingButtonPressed();

	UFUNCTION(BlueprintCallable)
	void StartCrouchToggle();

	UFUNCTION(BlueprintCallable)
	void FinishCrouchToggle();

	/** Handle interpolation for CapsuleHalfHeight */
	void HandleHalfHeightInterp(float DeltaTime) const;
	
	virtual void Jump() override;

	/** Called when LandStart animation notify gets triggered */
	UFUNCTION(BlueprintCallable)
	void StartLanding();
	
	/** Called when LandFinish animation notify gets triggered */
	UFUNCTION(BlueprintCallable)
	void FinishLanding();

	/** Add the ammo item to the inventory and consume it */
	void PickupAmmo(class AAmmo* Ammo);

	/** Create FInterpLocation structs for each location, and add them to the array */
	void InitializeInterpLocations();

	/** Handle adding items to the inventory by filling order */
	void AddToInventory(AWeapon* Weapon);
	
	/** Handle replacing items in the inventory for a specific index */
	void ReplaceInInventory(AWeapon* Weapon, int32 Index);
	
	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
	
	void EquipDefaultWeapon();
	void EquipWeaponOne();
	void EquipWeaponTwo();
	void EquipWeaponThree();
	void EquipWeaponFour();
	void EquipWeaponFive();

	void EquipNextWeapon();
	void EquipPreviousWeapon();

	/** Return the first empty slot in our inventory list */
	int32 GetEmptyInventorySlotIndex();
	
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

	/** Current speed of the character in X and Y direction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = "true"))
	float CurrentSpeed;

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
	
	/** Montage for firing weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;

	/** Montage for equipping weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipEquipMontage;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;
	
	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** True when aiming */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	/** Default camera field of view value */
	float CameraDefaultFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
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

	/** True when aiming button is pressed */
	bool bAimingButtonPressed;
	
	/** Right mouse button or right console trigger pressed */
	bool bFireButtonPressed;

	/** True when we can fire. False when waiting for the timer */
	bool bShouldFire;
	
	/** True while gun is firing */
	bool bFiringBullet;

	/** Duration of crosshair spread for shooting */
	float CrosshairShootingDuration;
	
	/** Sets a timer between gunshots */
	FTimerHandle AutomaticFireRateTimer;

	/** Sets a timer between crosshair spreads */
	FTimerHandle CrosshairShootTimer;

	/** True if we should trace items for every frame */
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

	/** Map to keep track of ammo of different ammo types */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	/** Starting amount of 9mm ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	/** Starting amount of 9mm ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	/** State of Character's combat */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	/** Montage for reloading the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat , meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	/** Transform of the clip when hand touches the clip for the first time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat , meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	/** Scene component to keep track of hand location with initial offset from the clip */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat , meta = (AllowPrivateAccess = "true"))
	USceneComponent* ClipSceneComponent;
	
	/** True when crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** True while toggling between crouch/stand */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouchToggling;

	/** Amount of speed for the character while standing (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float HipMovementSpeed;

	/** Amount of speed for the character while aiming (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AimingMovementSpeed;

	/** Amount of speed for the character while crouching (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/** Value of CapsuleHalfHeight while standing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float HipCapsuleHalfHeight;

	/** Value of CapsuleHalfHeight while crouching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchCapsuleHalfHeight;

	/** Value of CapsuleHalfHeight while crouching walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchWalkingCapsuleHalfHeight;

	/** Amount of speed for half height interpolation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CapsuleHalfHeightInterpSpeed;

	/** True when recovering from landing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLandRecovering;
	
	/** Amount of extra velocity for the character while jumping (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpBoostVelocity;

	/** The minimum speed required for being able to jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MinimumSpeedNeededForJumping;

	/** Amount of speed for the character while recovering from landing (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LandingRecoveryMovementSpeed;

	/** Location of the weapon when hovered over the camera during interpolation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;

	// Location of the item(s) when hovered over the camera during interpolation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	/** Array of interp location structs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	void RestPickupSoundTimer();
	void ResetEquipSoundTimer();

	/** Amount of time between each pickup sound */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundTimerDuration;

	/** Amount of time between each equip sound */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundTimerDuration;

	/** An array of AItems for our Inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	/* Delegate for sending slot information to Inventory bar when equipping */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	/** Delegate for highlighting the selected slot in the Inventory */
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = Inventory , meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	/** Slot index of the highlighted slot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlotIndex;
	
public:
	/** Returns CameraBoom subObject */
	FORCEINLINE USpringArmComponent *GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subObject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns current speed of the character in X and Y direction */
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	
	FORCEINLINE bool GetAiming() const { return bAiming; }

	/** Returns CrosshairSpreadingMultiplier function */
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
	
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FORCEINLINE bool GetFireButtonPressed() const { return bFireButtonPressed; }

	FORCEINLINE bool GetShouldPickupSound() const { return bShouldPlayPickupSound; }

	FORCEINLINE bool GetShouldEquipSound() const { return bShouldPlayEquipSound; };

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/** Add/subtract OverlappedItemCount and updates bShouldTraceForItems */
	void IncrementOverlappedItemCount(int8 Value);

	/** Get the desired interp location based on an Index in the array */
	FInterpLocation GetInterpLocation(int32 Index);

	/** Get the index of an InterpLocation with the lowest ItemCount */
	int32 GetInterpLocationIndex();

	/** Add/subtract ItemCount from desired InterpLocation */
	void IncrementInterpLocItemCount(int32 Index, int32 Amount);
	
	/** Set an item to pickup
	 * @param Item Item to pick up
	 */
	void PickupItem(AItem* Item);

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();

	void HighlightInventorySlot();
	void UnHighlightInventorySlot();
};