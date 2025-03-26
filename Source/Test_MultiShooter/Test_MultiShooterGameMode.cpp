// Copyright Epic Games, Inc. All Rights Reserved.

#include "Test_MultiShooterGameMode.h"
#include "Test_MultiShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATest_MultiShooterGameMode::ATest_MultiShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
