// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	// Item pickup interpolation variables
	ItemInterpStartLocation(FVector(0.f)),
	ItemInterpCameraTargetLocation(FVector(0.f)),
	bInterping(false),
	CurveDuration(0.7f),
	InterpInitialYawOffset(0.f)
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
	}
}

void AItem::StartAnimCurves(AShooterCharacter* Char)
{
	Character = Char; // Store a handle to the Character that picked the item
	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterp);
	
	GetWorldTimerManager().SetTimer(CurveTimer, this, &AItem::FinishAnimCurves, CurveDuration);

	const float CameraYaw = Character -> GetCameraBoom() -> GetComponentRotation().Yaw;
	const float ItemYaw = GetActorRotation().Yaw;
	
	// Offset between camera and item yaw
	InterpInitialYawOffset = ItemYaw - CameraYaw;
}

void AItem::FinishAnimCurves()
{
	bInterping = false;
	if(Character)
	{
		Character -> PickupItem(this);
	}
	if(ItemScaleCurve) SetActorScale3D(FVector(1.f)); // Set scale back to normal
}

void AItem::PickupInterpHandler(float DeltaTime)
{
	if(!bInterping) return;
	
	if(Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(CurveTimer);
		const float ZCurveValue = ItemZCurve -> GetFloatValue(ElapsedTime);

		FVector ItemCurrentLocation = ItemInterpStartLocation;
		const FVector TargetInterpLocation{ Character -> GetPickupInterpTargetLocation() };

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
			SetActorScale3D(FVector(ScaleCurveValue));
		}
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Handle item pickup interpolation when (bInterping = true)
	PickupInterpHandler(DeltaTime);
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	UpdateItemProperties(State);
}