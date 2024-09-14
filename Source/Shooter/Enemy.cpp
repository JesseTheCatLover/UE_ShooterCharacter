// Copyright 2024 JesseTheCatLover. All Rights Reserved.


#include "Enemy.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AEnemy::AEnemy():
MaxHealth(100.f),
Health(MaxHealth),
HeadBone("head"),
HealthBarDisplayDuration(4.f),
HitSectionName("HitReactFront"),
bCanHitReact(true),
HitReactDurationMin(0.5f),
HitReactDurationMax(0.75f),
HitNumberDestroyTime(1.5f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	HideHealthBar();
	GetMesh() -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, 
		HealthBarDisplayDuration);
}

void AEnemy::Die()
{
	HideHealthBar();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if(!bCanHitReact) return;
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if(AnimInstance && HitMontage)
	{
		AnimInstance -> Montage_Play(HitMontage, PlayRate);
		AnimInstance -> Montage_JumpToSection(Section);
	}

	bCanHitReact = false;
	const float HitReactDuration{ FMath::FRandRange(HitReactDurationMin, HitReactDurationMax) };
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactDuration);
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::DestroyHitNumberWidget(UUserWidget* HitNumberWidget)
{
	HitNumbers.Remove(HitNumberWidget);
	HitNumberWidget -> RemoveFromParent();
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumberWidget, FVector Location)
{
	HitNumbers.Add(HitNumberWidget, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumberWidget"), HitNumberWidget);

	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::UpdateHitLocation()
{
	for(auto &HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{ HitPair.Key };
		const FVector HitLocation{ HitPair.Value };

		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld() -> GetFirstPlayerController(), HitLocation, ScreenPosition);
		HitNumber -> SetPositionInViewport(ScreenPosition);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitLocation();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	ShowHealthBar();
	PlayHitMontage(FName(HitSectionName));
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}