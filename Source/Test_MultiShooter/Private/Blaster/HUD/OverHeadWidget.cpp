// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/HUD/OverHeadWidget.h"

#include "Components/TextBlock.h"

void UOverHeadWidget::SetDisplayText(FString Text)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(Text));
	}
}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole Role = InPawn->GetLocalRole();
	FString RoleString;
	switch (Role)
	{
	case ROLE_Authority:
		RoleString = FString(TEXT("Authority"));
		break;
	case ROLE_AutonomousProxy:
		RoleString = FString(TEXT("AutonomousProxy"));
		break;
	case ROLE_SimulatedProxy:
		RoleString = FString(TEXT("Simulated Proxy"));
		break;
	case ROLE_None:
		RoleString = FString(TEXT("None"));
		break;
	}
	SetDisplayText(RoleString);
}

void UOverHeadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
