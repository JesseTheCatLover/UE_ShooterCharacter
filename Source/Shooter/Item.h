// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterp UMETA(DisplayName = "EquipInterp"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	
	EIS_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	EIT_Ammo UMETA(DisplayName = "Ammo"),

	EIT_Max UMETA(DisplayName" DefaultMax")
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Set default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called when overlapping AreaSphere */
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	/** Called when overlapping AreaSphere ends */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Set the ActiveStars array of bools based on the rarity */
	void SetActiveStars();

	/** Set properties for Item's components based on the State */
	virtual void UpdateItemProperties(EItemState State);

	/** Called when item interpolation is finished */
	void FinishInterping();

	/** Handle item pickup interpolation when (bInterping = true) */
	void PickupInterpHandler(float DeltaTime);

	/** Get the desired location for Item pick up interpolation */
	bool GetPickupInterpTargetLocation(FVector &Location) const;

	void PlayPickupSound(bool bForcePlay = false) const;

	/** Initialize outline post-processing and assign its default value */
	virtual void InitializeCustomDepth();
	
	virtual void OnConstruction(const FTransform &Transform) override;
	
	void StartGlowPulseTimer();

	void ResetGlowPulseTimer();

	/** Handle item's GlowPulse */	
	void GlowPulseHandler() const;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Type of the Item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;
	
	/** Skeletal mesh for the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	/** Collision box for line tracing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	/** Widget to show when player is looking at the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	/** Sphere to calculate overlaps when you are close to the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	/** The name which appears on the Pickup widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/** Item count (ammo, ect.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/** Item rarity - determines number of stars in Pickup widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	/** Shown stars in Pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	/** AItem states for interactions */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	/** The curve asset to use for item's z location when interpolating */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	/** The location item starts interpolating for pickup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	/** The location item finished interpolation for pickup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpCameraTargetLocation;
	
	/** True when interpolating */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping;

	/** Timer to handle item pickup interpolation */
	FTimerHandle InterpCurveTimer;
	
	/** The point in timeline where interp curves stop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float InterpCurveDuration;
	
	/** Reference to AShooterCharacter to access its public functions */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	/** Initial yaw offset between the camera yaw and item yaw once the item is picked up*/
	float InterpInitialYawOffset;

	/** The curve asset to use for item's scale when interpolating (optional) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	/** Sound to play when picking item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	/** Sound to play when equipping item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	/** Index of the interp location this item is currently interping to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocationIndex;

	/** Scale of the item size while interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float InterpSizeScale;

	/** @param True = Outline post-processing on BeginPlay [enabled]
	 *  @param False = Outline post-processing on BeginPlay [disabled]
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool bCustomDepthOnBeginPlay;
	
	/** Glow material instance used for the Item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* GlowMaterialInstance;
	
	/** Index for the target material to be changed at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	int32 GlowMaterialIndex;

	/** Name of the Glow blend alpha parameter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	FName GlowBlendAlphaParameterName;

	/** Name of the Glow amount parameter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	FName GlowAmountParameterName;

	/** Name of the Fresnel exponent parameter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	FName FresnelExponentParameterName;

	/** Name of the Fresnel reflect fraction parameter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	FName FresnelReflectFractionParameterName;

	/** Value of GlowAmount factor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	/** Value of FresnelExponent factor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	/** Value of FresnelReflectFraction factor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;
	
	/** Dynamic instance that changes at runtime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* GlowMaterialInstanceDynamic;

	/** CurveVector for glow pulse */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	class UCurveVector* GlowPulseCurve;

	/** CurveVector for glow pulse while interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	UCurveVector* GlowPulseInterpCurve;

	/** Timer to handle glow pulse effect */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	FTimerHandle GlowPulseTimer;

	/** The point in timeline where GlowPulseCurve stops */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GlowMaterial, meta = (AllowPrivateAccess = "true"))
	float GlowPulseDuration;

	/** Item icon for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* ItemIcon;
	
	/** Rarity background for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* RarityIconBackground;
	
	/** AmmoType icon for this item in the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoTypeIcon;

	/** Current index position of the item in the inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** True when Character's inventory is full */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

public:	
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }
	
	/** Set new state for ItemState and calls UpdateItemProperties() */
	void SetItemState(EItemState State);
	
	/** Play PickupCurveTimer and call PickupInterpHandler() every frame
	 *	to handle pickup interpolation based on the curve values
	 *	@param Char This is a pointer to the player who is picking up the item */
	void StartPickingItem(AShooterCharacter* Char, bool bForcePlay = false);

	void PlayEquipSound(bool bForcePlay = false) const;

	void EnableGlowMaterial() const;
	
	void DisableGlowMaterial() const;
	
	/** Enable outline post-process */
	virtual void EnableCustomDepth();

	/** Disable outline post-process */
	virtual void DisableCustomDepth();
};
