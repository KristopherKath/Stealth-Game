// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	//Player Controller exists only on the Server and the client that owns this player controller. 
		//Difference with FPSCharacter is that character exists on all clients and server to keep in sync visually

	//This is implemented in Blueprint - Displays Mission Complete HUD
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
};
