// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void UpdateItemProperties(EItemState State) override;

	UFUNCTION()
	void OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
	USphereComponent* PickupSphere;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;
};