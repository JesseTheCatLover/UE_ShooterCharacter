// Copyright 2024 JesseTheCatLover. All Rights Reserved.


#include "Enemy.h"

#include "EnemyController.h"
#include "ShooterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AEnemy::AEnemy():
MaxHealth(400.f),
Health(0.f),
HitDamage(20.f),
HeadBone("head"),
HealthBarDisplayDuration(4.f),
HitSectionName("HitReactFront"),
bCanHitReact(true),
HitReactDurationMin(0.5f),
HitReactDurationMax(0.75f),
HitNumberDestroyTime(1.5f),
AttackRFast("AttackRFast"),
AttackR("AttackR"),
AttackLFast("AttackLFast"),
AttackL("AttackL"),
RightMeleeTipSocketName(TEXT("FX_Trail_R_01")),
LeftMeleeTipSocketName(TEXT("FX_Trail_L_01")),
bDying(false),
DeathDestroyDuration(10.f),
bStunned(false),
StunChance(0.2f),
bInAttackRange(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere -> SetupAttachment(GetRootComponent());

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere -> SetupAttachment(GetRootComponent());

	RightMeleeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightMeleeBox"));
	RightMeleeBox -> SetupAttachment(GetMesh(), "RightArmBone");
	LeftMeleeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftMeleeBox"));
	LeftMeleeBox -> SetupAttachment(GetMesh(), "LeftArmBone");
	
	GetCharacterMovement() -> MaxWalkSpeed = 500.f;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth; // Refill the health
	HideHealthBar();

	// Getting Blackboard ready
	const FVector WorldPatrolPointFirst = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPointFirst);
	const FVector WorldPatrolPointSecond = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPointSecond);
	EnemyController = Cast<AEnemyController>(GetController());
	if(EnemyController)
	{
		EnemyController -> GetBlackboardComponent() -> SetValueAsVector(TEXT("PatrolPointFirst"), WorldPatrolPointFirst);
		EnemyController -> GetBlackboardComponent() -> SetValueAsVector(TEXT("PatrolPointSecond"), WorldPatrolPointSecond);
		EnemyController -> GetBlackboardComponent() -> SetValueAsBool(TEXT("Dead"), false);
		EnemyController -> RunBehaviorTree(BehaviorTree);
	}

	// Setting up collision settings
	AgroSphere -> OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapped);
	AttackRangeSphere -> OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackSphereOverlapped);
	AttackRangeSphere -> OnComponentEndOverlap.AddDynamic(this, &AEnemy::AttackSphereOverlapEnd);
	RightMeleeBox -> OnComponentBeginOverlap.AddDynamic(this, &AEnemy::RightMeleeBoxOverlapped);
	LeftMeleeBox -> OnComponentBeginOverlap.AddDynamic(this, &AEnemy::LeftMeleeBoxOverlapped);
	
	GetMesh() -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh() -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent() -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	RightMeleeBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightMeleeBox -> SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightMeleeBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightMeleeBox -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	LeftMeleeBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftMeleeBox -> SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftMeleeBox -> SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftMeleeBox -> SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, 
		HealthBarDisplayDuration);
}

void AEnemy::Die()
{
	if(bDying) return;
	bDying = true;
	HideHealthBar();
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance -> Montage_Play(DeathMontage);
	}
	if(EnemyController)
	{
		EnemyController -> GetBlackboardComponent() -> SetValueAsBool(TEXT("Dead"), true);
		EnemyController -> StopMovement();
	}
}

void AEnemy::FinishDeath()
{
	GetMesh() -> bPauseAnims = true;
	GetWorldTimerManager().SetTimer(DeathDestroyTimer, this, &AEnemy::DestroyEnemy, DeathDestroyDuration);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
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

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	if(AnimInstance && AttackMontage)
	{
		AnimInstance -> Montage_Play(AttackMontage, PlayRate);
		AnimInstance -> Montage_JumpToSection(Section);
	}
}

FName AEnemy::GetRandomAttackSectionName()
{
	switch(FMath::RandRange(1, 4))
	{
	case 1:
		return AttackRFast;
	case 2:
		return AttackR;
	case 3:
		return AttackLFast;
	case 4:
		return AttackL;
	default:
		return AttackR;
	}
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

void AEnemy::AgroSphereOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		EnemyController -> GetBlackboardComponent() -> SetValueAsObject(TEXT("Target"), Character);
		GetCharacterMovement() -> MaxWalkSpeed = 600.f;
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if(EnemyController)
		EnemyController -> GetBlackboardComponent() -> SetValueAsBool(TEXT("Stunned"), Stunned);
}

void AEnemy::AttackSphereOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		bInAttackRange = true;
		if(EnemyController)
			EnemyController -> GetBlackboardComponent() -> SetValueAsBool(TEXT("InAttackRange"), true);
	}
}

void AEnemy::AttackSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		bInAttackRange = false;
		if(EnemyController)
			EnemyController -> GetBlackboardComponent() -> SetValueAsBool(TEXT("InAttackRange"), false);
	}
}

void AEnemy::RightMeleeBoxOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Victim = Cast<AShooterCharacter>(OtherActor);
	DoDamage(Victim);
	SpawnBlood(Victim, RightMeleeTipSocketName);
}

void AEnemy::LeftMeleeBoxOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Victim = Cast<AShooterCharacter>(OtherActor);
	DoDamage(Victim);
	SpawnBlood(Victim, LeftMeleeTipSocketName);
}

void AEnemy::ActivateRightMeleeCollision()
{
	RightMeleeBox -> SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightMeleeCollision()
{
	RightMeleeBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateLeftMeleeCollision()
{
	LeftMeleeBox -> SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftMeleeCollision()
{
	LeftMeleeBox -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DoDamage(AShooterCharacter* Victim)
{
	if(!Victim) return;
	UGameplayStatics::ApplyDamage(Victim, HitDamage, GetController(), this, UDamageType::StaticClass());
	if(MeleeHitImpactSound) UGameplayStatics::PlaySoundAtLocation(this, MeleeHitImpactSound, Victim -> GetActorLocation());
	StunVictim(Victim);
}

void AEnemy::SpawnBlood(AShooterCharacter* Victim, FName SocketName)
{
	if(Victim)
	{
		const USkeletalMeshSocket* TipSocket{ GetMesh() -> GetSocketByName(SocketName) };
		if(TipSocket)
		{
			const FTransform SocketTransform{ TipSocket -> GetSocketTransform(GetMesh()) };
			if(Victim -> GetBloodParticles())
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Victim -> GetBloodParticles(), SocketTransform);
			}
		}
	}
}

void AEnemy::StunVictim(AShooterCharacter* Victim)
{
	if(!Victim) return;
	const float Chance{ FMath::FRandRange(0.f, 1.f) };
	if(Chance <= Victim -> GetStunChance())
	{
		Victim -> Stun();
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
	if(bDying) return;
	
	ShowHealthBar();
	const float Stun = FMath::FRandRange(0.f, 1.f);
	if(Stun <= StunChance)
	{
		PlayHitMontage(FName(HitSectionName));
		SetStunned(true);
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if(BulletImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(EnemyController)
	{
		EnemyController -> GetBlackboardComponent() -> SetValueAsObject(TEXT("Target"), DamageCauser);
	}
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