// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "Ammo.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AAmmo::AAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	// Attach Item properties from SkeletonMesh to Mesh
	GetCollisionBox() -> SetupAttachment(GetRootComponent());
	GetPickupWidget() -> SetupAttachment(GetRootComponent());
	GetAreaSphere() -> SetupAttachment(GetRootComponent());

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollisionSphere"));
	PickupSphere -> SetupAttachment(GetRootComponent());
	PickupSphere -> SetSphereRadius(130.f);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	PickupSphere -> OnComponentBeginOverlap.AddDynamic(this, &AAmmo::OnPickupSphereOverlap);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::UpdateItemProperties(EItemState State)
{
	Super::UpdateItemProperties(State);

	switch(State)
	{
	case EItemState::EIS_Pickup:
		AmmoMesh -> SetSimulatePhysics(false);
		AmmoMesh -> SetEnableGravity(false);
		AmmoMesh -> SetVisibility(true);
		AmmoMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_EquipInterp:
		AmmoMesh -> SetSimulatePhysics(false);
		AmmoMesh -> SetEnableGravity(false);
		AmmoMesh -> SetVisibility(true);
		AmmoMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Equipped:
		AmmoMesh -> SetSimulatePhysics(false);
		AmmoMesh -> SetEnableGravity(false);
		AmmoMesh -> SetVisibility(true);
		AmmoMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		AmmoMesh -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh -> SetSimulatePhysics(true);
		AmmoMesh -> SetEnableGravity(true);
		AmmoMesh -> SetVisibility(true);
		AmmoMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh -> SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		break;

	case EItemState::EIS_PickedUp:
		AmmoMesh -> SetSimulatePhysics(false);
		AmmoMesh -> SetEnableGravity(false);
		AmmoMesh -> SetVisibility(false);
		AmmoMesh -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AAmmo::OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
	if(OverlappedCharacter)
	{
		StartPickingItem(OverlappedCharacter);
		PickupSphere -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAmmo::EnableCustomDepth()
{
	AmmoMesh -> SetRenderCustomDepth(true);	
}

void AAmmo::DisableCustomDepth()
{
	AmmoMesh -> SetRenderCustomDepth(false);
}