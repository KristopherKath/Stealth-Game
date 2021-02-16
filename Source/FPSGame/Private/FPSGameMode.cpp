// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameState.h"
#include "Engine/World.h"

AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();

	//Sets current gamemodes game state so that when game ends it can set player information
	GameStateClass = AFPSGameState::StaticClass();
}

//This will set the players to transition scene and fire GameState netmulticast to set their information for the game over sequence
void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if (InstigatorPawn) {

		if (SpectatingViewpointClass)
		{
			TArray<AActor*> ReturnedActors;
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);

			//Change viewtarget if any valid actor found
			if (ReturnedActors.Num() > 0)
			{
				AActor* NewViewTarget = ReturnedActors[0];
				
				//Get all player controllers 
				//For loop for setting the camera transition for game complete for all clients and server
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					APlayerController* PC = It->Get();
					//We dont check if this is a local controller because we want to be called on all players that the server controls
					if (PC)
					{
						PC->SetViewTargetWithBlend(NewViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatingViewportClass is nullptr. Please update GameMode class with valid subclass. Cannot change spectating view target."));
		}
	}

	//Sets new game state then checks if we should disable input and show mission complete
	AFPSGameState* GS = GetGameState<AFPSGameState>();
	if (GS)
	{
		//Using the GameState class we use netmulticast to change the players end game message and disable their input
		GS->MulticastOnMissionComplete(InstigatorPawn, bMissionSuccess);
	}

	//Send information to 
	OnMissionCompleted(InstigatorPawn, bMissionSuccess);
}
