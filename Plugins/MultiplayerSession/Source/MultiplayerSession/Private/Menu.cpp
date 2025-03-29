// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

void UMenu::MenuSetUP()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeUI;
			InputModeUI.SetWidgetToFocus(TakeWidget());
			InputModeUI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeUI);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}
