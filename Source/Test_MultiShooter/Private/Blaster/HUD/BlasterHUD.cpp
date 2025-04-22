// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/HUD/BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		if (HUDPackage.CrossHairs)
		{
			
			FVector2D ViewportSize;
			GEngine->GameViewport->GetViewportSize(ViewportSize);
			FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

			const float TextureWidth = HUDPackage.CrossHairs->GetSizeX();
			const float TextureHeight = HUDPackage.CrossHairs->GetSizeY();
			const FVector2D TextureDrawPoint(
				ViewportCenter.X - (TextureWidth / 2.f),
				ViewportCenter.Y - (TextureHeight / 2.f)
			);

			
			DrawTexture(HUDPackage.CrossHairs,
				TextureDrawPoint.X,
				TextureDrawPoint.Y,
				TextureWidth,
				TextureHeight,
				0.f,
				0.f,
				1.f,
				1.f,
				HUDPackage.CrossHairsColor
				);
		}
	}
}
