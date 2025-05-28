// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS(minimalapi)
class ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ALobbyGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	virtual void Logout(AController* Exiting) override;
	
};



