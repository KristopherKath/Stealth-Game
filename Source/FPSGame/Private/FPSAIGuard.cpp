// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "FPSGameMode.h"
#include "AIController.h"
#include "Classes/Blueprint/AIBlueprintHelperLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnPawnHeard);

	GuardState = EAIState::Idle;
}


// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();

	OriginalRotation = GetActorRotation();

	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}


/* Method for when Guard sees Player */
void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if (SeenPawn == nullptr) { return; }

	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);

	//Stop Movement if Patroling
	AController* Controller = GetController();
	if (Controller)
	{
		Controller->StopMovement();
	}
}


/* Edit Guard direction based on noise heard */
void AFPSAIGuard::OnPawnHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	//If Guard state is alerted we dont need noises
	if (GuardState == EAIState::Alerted) { return; }
	SetGuardState(EAIState::Suspicious);
	
	//Debug
	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	//get direction to look at
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	//Make Guard look at new direction without changing Z axis
	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;
	SetActorRotation(NewLookAt);

	//Timer for how long to look before looking back at original dir
		//Reset timer if one was already made
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
		//Make new timer
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	//Stop Movement if Patroling
	AController* Controller = GetController();
	if (Controller)
	{
		Controller->StopMovement();
	}
}


/* Method for reseting Guard orientation */
void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted) { return; }
	SetGuardState(EAIState::Idle);

	SetActorRotation(OriginalRotation);

	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}


void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

/* Sets the Guard State */
void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	//If already set to this state then don't set
	if (GuardState == NewState) { return; }

	//Set state
	GuardState = NewState;
	OnRep_GuardState();
}


void AFPSAIGuard::MoveToNextPatrolPoint()
{
	//Assign next patrol point
	if (CurrentPatrolPoint == nullptr || CurrentPatrolPoint == SecondPatrolPoint) {
		CurrentPatrolPoint = FirstPatrolPoint;
	}
	else {
		CurrentPatrolPoint = SecondPatrolPoint;
	}

	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
}


// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPatrolPoint)
	{
		FVector Delta = GetActorLocation() - CurrentPatrolPoint->GetActorLocation();
		float DistanceToGoal = Delta.Size();

		//Check if we are within 50 units of goal, if so pick new point
		if (DistanceToGoal < 70)
		{
			MoveToNextPatrolPoint();
		}
	}
}

//This replicated to all clients
void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}


