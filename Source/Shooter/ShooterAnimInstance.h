// Copyright 2023 JesseTheCatLover. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimation(float DeltaTime);

    virtual void NativeInitializeAnimation() override;
protected:

	/** Update properties, such as Speed, bAiming, bFiring... (ect) */
	void UpdateAnimationProperties();

	void UpdateIsIdle();
	void UpdateRecoilWeight();
	
	/** Handle calculations for strafing */
	void Strafe();
	
	/** Handle turn in place calculations */
	void TurnInPlace(float DeltaTime);

	/** Handle calculations for leaning */
	void Lean(float DeltaTime);
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The Speed of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Whether or not the character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** Whether or not the character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** Offset yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before me stopped moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bFiring;

	/** Yaw of the character updated for TurnInPlace function */
	float TIPCharacterYaw;

	/** Yaw of the character in the previous frame for TurnInPlace function */
	float TIPCharacterYawPreviousFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation curve value */
	float RotationCurve;

	/** Rotation curve value previous frame */
	float RotationCurvePreviousFrame;

	/** Pitch value for AimingOffset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in place", meta = (AllowPrivateAccess = "true"))
	float CharacterCurrentPitch;

	/** True when reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/** Handle the states for AimingOffset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	/** True when turning */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn in place", meta = (AllowPrivateAccess = "true"))
	bool bTurning;

	/** Rotation of the character */
	FRotator CharacterRotation;

	/** Rotation of the character in the previous frame */
	FRotator CharacterRotationPreviousFrame;

	/** Difference between the yaw of character in the previous frame from current yaw */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float CharacterYawDelta;

	/** True when crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** True when equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	/** True when taking no action */
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsIdle;

	/** True when the IdleCoolDown has finished.(Used for triggering idle animation event) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCanIdle;

	/** Duration to start the Idle event since the last given input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float IdleCoolDown;

	/** Recoil of the weapon based on different state poses */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	/** Interp speed for turning while jugging from idle.(Between 13f-25f) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "13.0", ClampMax = "25.0", UIMin = "13.0", UIMax = "25.0"))
	float JogTurningInterpSpeed;
};
