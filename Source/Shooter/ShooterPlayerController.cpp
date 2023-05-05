// Copyright 2023 JesseTheCatLover. All Rights Reserved.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController()
{
	
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		
		if(HUDOverlay)
		{
			HUDOverlay -> AddToViewport();
			HUDOverlay -> SetVisibility(ESlateVisibility::Visible);
		}
	}
}

