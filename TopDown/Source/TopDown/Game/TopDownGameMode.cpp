// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownGameMode.h"
#include "TopDown/Game/TopDownPlayerController.h"
#include "TopDown/Character/TopDownCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATopDownGameMode::ATopDownGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATopDownPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprint/Character/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprint/Character/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}