// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance():
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	bFiring(false),
	TIPCharacterYaw(0.f),
	TIPCharacterYawPreviousFrame(0.f),
	RootYawOffset(0.f),
	CharacterCurrentPitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	bTurning(false),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationPreviousFrame(FRotator(0.f)),
	CharacterYawDelta(0.f),
	bCrouching(false),
	bIsIdle(true),
	bCanIdle(true),
	IdleCoolDown(7.f),
	RecoilWeight(1.f),
	JogTurningInterpSpeed(15.f)
{
	
}

void UShooterAnimInstance::UpdateAnimation(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	// Update properties
	UpdateAnimationProperties();

	// Animation mechanics
	Strafe();
	TurnInPlace(DeltaTime);
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties()
{
	if(ShooterCharacter == nullptr) return;

	// Get properties from character class
	Speed = ShooterCharacter -> GetCurrentSpeed();
	bAiming = ShooterCharacter -> GetAiming();
	bFiring = ShooterCharacter -> GetFireButtonPressed();
	bCrouching = ShooterCharacter -> GetCrouching();
	bReloading = ShooterCharacter -> GetCombatState() == ECombatState::ECS_Reloading;
	bIsInAir = ShooterCharacter -> GetCharacterMovement() -> IsFalling();
	
	// Is the character accelerating?
	if(ShooterCharacter -> GetCharacterMovement() -> GetCurrentAcceleration().Size() > 0.f)
	{
		bIsAccelerating = true;
	}
	else
	{
		bIsAccelerating = false;
	}

	// OffsetState
	if(bIsInAir) OffsetState = EOffsetState::EOS_InAir;
	else if(bAiming) OffsetState = EOffsetState::EOS_Aiming;
	else if(bReloading) OffsetState = EOffsetState::EOS_Reloading;
	else OffsetState = EOffsetState::EOS_Hip;
	
	UpdateIsIdle();
	UpdateRecoilWeight();
}

void UShooterAnimInstance::UpdateIsIdle()
{
	if(Speed > 0) bIsIdle = false;
	else if(bIsInAir) bIsIdle = false;
	else if(bAiming) bIsIdle = false;
	else if(bFiring) bIsIdle = false;
	else if(bReloading) bIsIdle = false;
	else if(bTurning) bIsIdle = false;
	else if(bCrouching) bIsIdle = false;
	else bIsIdle = true;
}

void UShooterAnimInstance::UpdateRecoilWeight()
{
	if(bTurning)
	{
		// No recoil while turning
		RecoilWeight = bReloading ? 1.f : 0.f;
	}
	else // not turning
		{
		if(bCrouching)
		{
			// Lower recoil while crouching
			RecoilWeight = bReloading ? 1.f : 0.1f;
		}
		else // Standing
			{
			RecoilWeight = bAiming || bReloading ? 1.f : 0.5f;
			}
		}
}

void UShooterAnimInstance::Strafe()
{
	if(ShooterCharacter == nullptr) return;

	// Calculating MovementOffsetYaw for strafing
	FRotator AimRotation = ShooterCharacter -> GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter -> GetVelocity());
	MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	if(ShooterCharacter -> GetVelocity().Size() > 0.f)
	{
		LastMovementOffsetYaw = MovementOffsetYaw;
	}
}

void UShooterAnimInstance::TurnInPlace(float DeltaTime)
{
	if(ShooterCharacter == nullptr) return;
	if(Speed > 0 || bIsInAir)
	{
		// We don't want to turn in place when we are moving or floating in air;
		// but we need to update and zero out some of the variables.
		bTurning = false;
		// Interp between standing yaw and walking forward yaw value
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaTime,
			FMath::Abs(RootYawOffset) > 40 ? JogTurningInterpSpeed - 12.f : JogTurningInterpSpeed);
		const float TargetPitch = ShooterCharacter -> GetBaseAimRotation().Pitch; // Target for hand pitch to follow
		// To give character's holding hand a realistic feeling of holding a gun while jogging
		CharacterCurrentPitch = FMath::FInterpTo(CharacterCurrentPitch, TargetPitch, DeltaTime, JogTurningInterpSpeed);
		TIPCharacterYaw = ShooterCharacter -> GetActorRotation().Yaw;
		TIPCharacterYawPreviousFrame = TIPCharacterYaw;
		RotationCurvePreviousFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{	
		// Getting the pitch of controller(Cursor)
		CharacterCurrentPitch = ShooterCharacter -> GetBaseAimRotation().Pitch;
		
		TIPCharacterYawPreviousFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter -> GetActorRotation().Yaw;
		const float TIPCharacterYawDelta{ TIPCharacterYaw - TIPCharacterYawPreviousFrame };

		// Clamp RootYawOffset between [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis( RootYawOffset - TIPCharacterYawDelta );

		// 1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if(Turning > 0)
		{
			bTurning = true;
			RotationCurvePreviousFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurvePreviousFrame };
			
			// RootYawOffset > 0, -> Turning left; RootYawOffset < 0, -> Turning right.
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;
			
			// Check to see if we aren't rotating too much
			const float ABSRootYawOffset = FMath::Abs(RootYawOffset);
			if(ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else 
		{
			bTurning = false;
		}
	}
	// Instantly check right after turning so we will have a smooth transition between Idle animation and turning
	UpdateIsIdle();
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if(ShooterCharacter == nullptr) return;
	if(Speed == 0) // if not moving
	{
		// Zero out CharacterYawDelta smoothly
		const float Interp{ FMath::FInterpTo(CharacterYawDelta, 0.0f, DeltaTime, 10.f) };
		CharacterYawDelta = FMath::Clamp(Interp, -85.f, 85.f); return;
	}

	CharacterRotationPreviousFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter -> GetActorRotation();

	const FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator
	(CharacterRotation, CharacterRotationPreviousFrame);

	const float Target = (RotationDelta.Yaw / DeltaTime);
	const float Interp{ FMath::FInterpTo(CharacterYawDelta, Target, DeltaTime, 1.f) };

	CharacterYawDelta = FMath::Clamp(Interp, -85.f, 85.f);
}