// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


class UAnnouncement;
class UCharacterOverlay;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	TObjectPtr<UTexture2D> CrossHairs;

	float CrosshairSpread;
	FLinearColor CrossHairsColor;
};

/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	TObjectPtr<UCharacterOverlay> CharacterOverlay;

	void AddCharacterOverlay();
	
	UPROPERTY(EditAnywhere, Category="Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	TObjectPtr<UAnnouncement> Announcement;

	void AddAnnouncement();
	
	virtual void DrawHUD() override;

	
protected:
	virtual void BeginPlay() override;
	
private:
	FHUDPackage HUDPackage;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

};
