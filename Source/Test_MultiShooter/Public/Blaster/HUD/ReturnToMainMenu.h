// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

class UMultiplayerSessionsSubsystem;
class UButton;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void MenuSetup();
	void MenuTearDown();
	
protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UFUNCTION()
	void OnPlayerLeftGame();

	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;
	
};
