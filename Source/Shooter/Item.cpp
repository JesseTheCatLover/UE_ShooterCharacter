// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

// Sets default values
AItem::AItem():
	ItemType(EItemType::EIT_Weapon),
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	// Item pickup interpolation variables
	ItemInterpStartLocation(FVector(0.f)),
	ItemInterpCameraTargetLocation(FVector(0.f)),
	bInterping(false),
	InterpCurveDuration(0.7f),
	InterpInitialYawOffset(0.f),
	InterpLocationIndex(0),
	InterpSizeScale(1.f),
	// Glow material variables
	bCustomDepthOnBeginPlay(false),
	GlowMaterialIndex(0),
	GlowBlendAlphaParameterName(TEXT("GlowBlendAlpha")),
	GlowAmountParameterName(TEXT("GlowAmount")),
	FresnelExponentParameterName(TEXT("FresnelExponent")),
	FresnelReflectFractionParameterName(TEXT("FresnelReflectFraction")),
	GlowAmount(150.f),
	FresnelExponent(3.f),
	FresnelReflectFraction(4.f),
	GlowPulseDuration(5.f),
	// Inventory
	SlotIndex(0),
	bCharacterInventoryFull(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisonBox"));
	CollisionBox -> SetupAttachment(ItemMesh);
	// Set CollisionBox to ignore all the channels expect ECC_Visibility channel, and block it.
	CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget -> SetupAttachment(ItemMesh);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere -> SetupAttachment(ItemMesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	if(PickupWidget)
	{
		// Hide the Pickup widget by default
		PickupWidget -> SetVisibility(false);
	}
	// Set ActiveStars array based on item rarity
	SetActiveStars();
	// Setup overlap for AreaSphere
	AreaSphere -> OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	AreaSphere -> OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	// Set properties for Item's components based on the state
	UpdateItemProperties(ItemState);

	// Initialize outline post-process
	InitializeCustomDepth();

	// Start glowing
	StartGlowPulseTimer();
}

void AItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter)
		{
			ShooterCharacter -> IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if(OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter)
		{
			ShooterCharacter -> IncrementOverlappedItemCount(-1);
			ShooterCharacter -> UnHighlightInventorySlot();
		}
	}
}

void AItem::SetActiveStars()
{
	for(int32 i = 0; i <= 5; i++) // Element 0 isn't used.
		ActiveStars.Add(false);
	
	switch(ItemRarity)
	{
	case EItemRarity::EIR_Legendary:
		ActiveStars[5] = true;
	case EItemRarity::EIR_Rare:
		ActiveStars[4] = true;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[3] = true;
	case EItemRarity::EIR_Common:
		ActiveStars[2] = true;
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true; break;
	}
}

void AItem::UpdateItemProperties(EItemState State)
{
	switch(State)
	{
	case EItemState::EIS_Pickup:
		ItemMesh -> SetSimulatePhysics(false);
		ItemMesh -> SetEnableGravity(false);
		ItemMesh -> SetVisibility(true);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		break;
	case EItemState::EIS_EquipInterp:
		PickupWidget -> SetVisibility(false);
		ItemMesh -> SetSimulatePhysics(false);
		ItemMesh -> SetEnableGravity(false);
		ItemMesh -> SetVisibility(true);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Equipped:
		PickupWidget -> SetVisibility(false);
		ItemMesh -> SetSimulatePhysics(false);
		ItemMesh -> SetEnableGravity(false);
		ItemMesh -> SetVisibility(true);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh -> SetSimulatePhysics(true);
		ItemMesh -> SetEnableGravity(true);
		ItemMesh -> SetVisibility(true);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		ItemMesh -> SetSimulatePhysics(false);
		ItemMesh -> SetEnableGravity(false);
		ItemMesh -> SetVisibility(false);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AItem::StartPickingItem(AShooterCharacter* Char, bool bForcePlay)
{
	Character = Char; // Store a handle to the Character that picked the item

	InterpLocationIndex = Character -> GetInterpLocationIndex();
	// Add 1 to the ItemCount for this interpolation struct 
	Character -> IncrementInterpLocItemCount(InterpLocationIndex, 1);
	
	PlayPickupSound(true);
	
	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterp);

	GetWorldTimerManager().ClearTimer(GlowPulseTimer);
	GetWorldTimerManager().SetTimer(InterpCurveTimer, this, &AItem::FinishInterping, InterpCurveDuration);

	const float CameraYaw = Character -> GetCameraBoom() -> GetComponentRotation().Yaw;
	const float ItemYaw = GetActorRotation().Yaw;
	
	// Offset between camera and item yaw
	InterpInitialYawOffset = ItemYaw - CameraYaw;
}

void AItem::FinishInterping()
{
	bInterping = false;
	GetWorldTimerManager().ClearTimer(InterpCurveTimer);
	
	if(Character)
	{
		Character -> PickupItem(this);
		// Subtract 1 from the ItemCount of this InterpLocation struct
		Character -> IncrementInterpLocItemCount(InterpLocationIndex, -1);
		Character -> UnHighlightInventorySlot();
	}
	if(ItemScaleCurve) SetActorScale3D(FVector(1.f)); // Set scale back to normal

	DisableGlowMaterial();
	DisableCustomDepth();
}

void AItem::PickupInterpHandler(float DeltaTime)
{
	if(!bInterping) return;
	
	if(Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(InterpCurveTimer);
		const float ZCurveValue = ItemZCurve -> GetFloatValue(ElapsedTime);

		FVector ItemCurrentLocation = ItemInterpStartLocation;
		FVector TargetInterpLocation{FVector(0.f)};
		if(!GetPickupInterpTargetLocation(TargetInterpLocation)) return;
		
		// Vector from item to camera, X and Y are zeroed out 
		const FVector ItemToCameraDeltaZ{ FVector(0.f, 0.f, (TargetInterpLocation - ItemCurrentLocation).Z ) };
		// Scale factor to multiply with the CurveValue
		const float DeltaZSize = ItemToCameraDeltaZ.Size();

		const FVector ItemFirstLocation{ GetActorLocation() };
		const float InterpXValue = FMath::FInterpTo(ItemFirstLocation.X, TargetInterpLocation.X,
			DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(ItemFirstLocation.Y, TargetInterpLocation.Y,
			DeltaTime, 30.f);

		// Set X and Y location of item to interpolated values
		ItemCurrentLocation.X = InterpXValue;
		ItemCurrentLocation.Y = InterpYValue;
		
		// Adding to the Z component of item location based on the CurveValue multiplied by DeltaZSize(Scale)
		ItemCurrentLocation.Z += ZCurveValue * DeltaZSize;
		SetActorLocation(ItemCurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// Camera's yaw this frame
		const float CurrentCameraYaw = Character -> GetCameraBoom() -> GetComponentRotation().Yaw;
		const FRotator ItemRotation{ 0.f, CurrentCameraYaw + InterpInitialYawOffset, 0.f };

		// Keeping the OffsetYaw between the item and camera constant
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if(ItemScaleCurve) // Applying a ScaleCurve is optional
		{
			const float ScaleCurveValue = ItemScaleCurve -> GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue) * FVector(InterpSizeScale));
		}
	}
}

bool AItem::GetPickupInterpTargetLocation(FVector &Location) const
{
	if(Character == nullptr) return false;

	switch(ItemType)
	{
	case EItemType::EIT_Weapon:
		Location = Character -> GetInterpLocation(0).SceneComponent -> GetComponentLocation();
		return true;
	case EItemType::EIT_Ammo:
		Location = Character -> GetInterpLocation(InterpLocationIndex).SceneComponent -> GetComponentLocation();
		return true;
	case EItemType::EIT_Max: break;
	}
	return false;
}

void AItem::PlayPickupSound(bool bForceplay) const
{
	if(Character)
	{
		if(bForceplay)
       	{
       		if(PickupSound)
      		{
       			UGameplayStatics::PlaySound2D(this, PickupSound);
       		}
   		}
       	else if(Character -> GetShouldPickupSound())
        {
        	if(PickupSound)
        	{
        		UGameplayStatics::PlaySound2D(this, PickupSound);
        	}
       	}
	}
}

void AItem::EnableGlowMaterial() const
{
	if(GlowMaterialInstanceDynamic)
	{
		GlowMaterialInstanceDynamic -> SetScalarParameterValue(GlowBlendAlphaParameterName, 0.f);
	}
}

void AItem::DisableGlowMaterial() const
{
	if(GlowMaterialInstanceDynamic)
	{
		GlowMaterialInstanceDynamic -> SetScalarParameterValue(GlowBlendAlphaParameterName, 1.f);
	}
}

void AItem::OnConstruction(const FTransform& Transform)
{
	// Load the data in the ItemRarityDataTable
	LoadRarityData();

	if(GlowMaterialInstance)
	{
		GlowMaterialInstanceDynamic = UMaterialInstanceDynamic::Create(GlowMaterialInstance, this);
		GlowMaterialInstanceDynamic -> SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemMesh -> SetMaterial(GlowMaterialIndex, GlowMaterialInstanceDynamic);
		EnableGlowMaterial();
	}
}

void AItem::StartGlowPulseTimer()
{
	if(ItemState == EItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(GlowPulseTimer, this,
			&AItem::ResetGlowPulseTimer, GlowPulseDuration);
	}
}

void AItem::ResetGlowPulseTimer()
{
	StartGlowPulseTimer();
}

void AItem::GlowPulseHandler() const
{
	if(ItemState == EItemState::EIS_Equipped) return;
	
	float ElapsedTime{};
	FVector GlowPulseVector{};
	
	switch(ItemState) // Pick the proper curve to read data from
	{
	case EItemState::EIS_Pickup:
		if(GlowPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(GlowPulseTimer);
			GlowPulseVector = GlowPulseCurve -> GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_EquipInterp:
		if(GlowPulseInterpCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(InterpCurveTimer);
			GlowPulseVector = GlowPulseInterpCurve ->GetVectorValue(ElapsedTime);
		}
		break;
	}

	if(GlowMaterialInstanceDynamic) // Assign the values to GlowPulse material instance
	{
		GlowMaterialInstanceDynamic -> SetScalarParameterValue(GlowAmountParameterName,
			GlowPulseVector.X * GlowAmount);
		GlowMaterialInstanceDynamic -> SetScalarParameterValue(FresnelExponentParameterName,
			GlowPulseVector.Y * FresnelExponent);
		GlowMaterialInstanceDynamic -> SetScalarParameterValue(FresnelReflectFractionParameterName,
			GlowPulseVector.Z * FresnelReflectFraction);
	}
}

void AItem::LoadRarityData()
{
	if(!ItemRarityDataTable) return;
	
	FItemRarityTable* RarityRow = nullptr;
	switch(ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		RarityRow = ItemRarityDataTable -> FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
		break;
	case EItemRarity::EIR_Common:
		RarityRow = ItemRarityDataTable -> FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
		break;
	case EItemRarity::EIR_Uncommon:
		RarityRow = ItemRarityDataTable -> FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
		break;
	case EItemRarity::EIR_Rare:
		RarityRow = ItemRarityDataTable -> FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
		break;
	case EItemRarity::EIR_Legendary:
		RarityRow = ItemRarityDataTable -> FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
		break;
	}
	if(RarityRow)
	{
		GlowColor = RarityRow -> GlowColor;
		WidgetColorLight = RarityRow -> WidgetColorLight;
		WidgetColorDark = RarityRow -> WidgetColorDark;
		NumberOfStars = RarityRow -> NumberOfStars;
		IconBackgroundRarity = RarityRow -> IconBackgroundRarity;
		if(GetItemMesh()) GetItemMesh() -> SetCustomDepthStencilValue(RarityRow -> CustomDepthStencil);
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Handle item pickup interpolation when (bInterping = true)
	PickupInterpHandler(DeltaTime);
	// Handle glow pulsation for the item
	GlowPulseHandler();
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	UpdateItemProperties(State);
}

void AItem::PlayEquipSound(bool bForcePlay) const
{
	if(Character)
	{
		if(bForcePlay)
		{
			if(EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
		else if(Character -> GetShouldEquipSound())
		{
			if(EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
	}
}

void AItem::InitializeCustomDepth()
{
	bCustomDepthOnBeginPlay ? EnableCustomDepth() : DisableCustomDepth();
}

void AItem::EnableCustomDepth()
{
	ItemMesh -> SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	if(!bInterping)
	{
		ItemMesh -> SetRenderCustomDepth(false);
	}
}
