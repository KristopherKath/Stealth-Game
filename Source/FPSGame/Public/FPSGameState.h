// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSGameState.generated.h"

//This is a companion class to Gamemode
//Helps disable all clients when game is over
	//Cannot be implemented in Gamemode since that class only
	//exists in the server and no client can run the complete misson procedure
UCLASS()
class FPSGAME_API AFPSGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	//This class is made so that we can send the game state to all clients since the GameMode class is only avaliable on the server so replication is impossible in that class

	//If this were to be called by server it would be sent to all clients and server (NetMulticast) - An RPC or Remote Procedure Call
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnMissionComplete(APawn* InstigatorPawn, bool bMissionSuccess);
};
