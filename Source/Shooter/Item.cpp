// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup)
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
		ItemMesh -> SetEnableGravity(true);
		ItemMesh -> SetVisibility(true);
		ItemMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Equipped:
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

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	UpdateItemProperties(State);
}
