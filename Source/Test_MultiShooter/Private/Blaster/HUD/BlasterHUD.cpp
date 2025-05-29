// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/HUD/BlasterHUD.h"

#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	AddElimAnnouncement("player1", "player2");
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController and CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = TObjectPtr<APlayerController>(GetOwningPlayerController());
	if (PlayerController and AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnnouncement(FString AttackerName, FString VictimName)
{
	OwningPlayerController = OwningPlayerController == nullptr? TObjectPtr<APlayerController>(GetOwningPlayerController()) : OwningPlayerController;
	if (OwningPlayerController and ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		if (ElimAnnouncement)
		{
			ElimAnnouncement->SetElimAnnouncementText(AttackerName, VictimName);
			ElimAnnouncement->AddToViewport();

			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			ElimMessages.Add(ElimAnnouncement);

			
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncement);
			GetWorldTimerManager().SetTimer(ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false);
			
		}
	}
	
}

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
