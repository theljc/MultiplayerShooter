// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class TEST_MULTISHOOTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> WarmUpTime;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AnnoucementText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> InfoText;
	
};
