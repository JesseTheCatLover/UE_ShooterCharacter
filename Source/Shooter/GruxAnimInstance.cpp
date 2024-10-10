// Copyright 2025 JesseTheCatLover. All Rights Reserved.


#include "GruxAnimInstance.h"
#include "Enemy.h"

void UGruxAnimInstance::UpdateAnimation(float DeltaTime)
{
	if(!Enemy)
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	
	if(Enemy)
	{
		FVector Velocity = Enemy -> GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
