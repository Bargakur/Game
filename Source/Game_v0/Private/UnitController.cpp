#include "UnitController.h"
#include "UnitBase.h"
#include "UnitCommand.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "Engine/World.h"


AUnitController::AUnitController()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasDestination = false;
    bIsMoving = false;
    CurrentDestination = FVector::ZeroVector;
    AcceptanceRadius = 4.0f;
    
    // Enable AI for this controller
    bWantsPlayerState = false;
}

void AUnitController::BeginPlay()
{
    Super::BeginPlay();
    
    ControlledUnit = Cast<AUnitBase>(GetPawn());
    if (!ControlledUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("UnitController: No valid UnitBase pawn assigned!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UnitController: Successfully got controlled unit: %s"), *ControlledUnit->GetName());
    }

}

void AUnitController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bHasDestination && ControlledUnit)
    {
        UpdateMovement(DeltaTime);
    }
}

void AUnitController::MoveToLocation(FVector Destination)
{
    if (!ControlledUnit)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnitController: Cannot move - no controlled unit"));
        return;
    }

    CurrentDestination = Destination;
    bHasDestination = true;
    bIsMoving = true;
    
    // Disable direct movement, enable pathfinding
    bUseDirectMovement = false;

    // Call AIController’s built-in pathfinding
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Destination);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
    MoveRequest.SetUsePathfinding(true);

    FNavPathSharedPtr NavPath;
    MoveTo(MoveRequest, &NavPath);

    // Trigger walking animation
    ControlledUnit->SetIsMoving(true);

    UE_LOG(LogTemp, Log, TEXT("UnitController: Pathfinding to location %s"), *CurrentDestination.ToString());
}

void AUnitController::StopMovement()
{
    Super::StopMovement(); // important to stop AIController pathfollowing

    bHasDestination = false;
    bIsMoving = false;
    bUseDirectMovement = false;
    
    if (ControlledUnit)
    {
        ControlledUnit->SetIsMoving(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UnitController: Stopped unit movement"));
}

void AUnitController::ExecuteCommand(const FUnitCommand& Command)
{
    switch (Command.CommandType)
    {
        case EUnitCommandType::Move:
            MoveToLocation(Command.TargetLocation);
            break;
        case EUnitCommandType::Stop:
            StopMovement();
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("UnitController: Unknown command type"));
            break;
    }
}

void AUnitController::UpdateMovement(float DeltaTime)
{
    if (!ControlledUnit || !bHasDestination)
        return;

    FVector CurrentLocation = ControlledUnit->GetActorLocation();
    float DistanceToDestination = FVector::Dist2D(CurrentLocation, CurrentDestination);

    // Check if we've reached the destination
    if (DistanceToDestination <= AcceptanceRadius)
    {
        OnReachedDestination();
        return;
    }
    
    // If using direct movement
    if (bUseDirectMovement)
    {
        MoveDirectly(DeltaTime);
    }
    
    // Debug logging
    UE_LOG(LogTemp, VeryVerbose, TEXT("Distance to destination: %f"), DistanceToDestination);
}

void AUnitController::MoveDirectly(float DeltaTime)
{
    if (!ControlledUnit)
        return;
        
    FVector CurrentLocation = ControlledUnit->GetActorLocation();
    FVector Direction = (CurrentDestination - CurrentLocation).GetSafeNormal();
    
    // Get movement component and use AddMovementInput for proper animation
    UCharacterMovementComponent* MovementComp = ControlledUnit->GetCharacterMovement();
    if (MovementComp)
    {
        // Use AddMovementInput for proper CharacterMovement integration
        ControlledUnit->AddMovementInput(Direction, 1.0f);
        
        // Rotate to face movement direction
        FRotator CurrentRotation = ControlledUnit->GetActorRotation();
        FRotator TargetRotation = Direction.Rotation();
        TargetRotation.Pitch = 0.0f; // Keep character upright
        
        // Smoothly rotate towards target
        float RotationSpeed = 540.0f; // degrees per second
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed / 90.0f);
        ControlledUnit->SetActorRotation(NewRotation);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Direct movement: Current %s, Target %s, Direction %s"), 
           *CurrentLocation.ToString(), *CurrentDestination.ToString(), *Direction.ToString());
}

void AUnitController::OnReachedDestination()
{
    bHasDestination = false;
    bIsMoving = false;
    bUseDirectMovement = false;
    
    if (ControlledUnit)
    {
        ControlledUnit->SetIsMoving(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UnitController: Unit reached destination"));
}


    void AUnitController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
    {
        Super::OnMoveCompleted(RequestID, Result);

        if (Result.IsSuccess())
        {
            OnReachedDestination();
        }
        else
        {
            StopMovement();
            UE_LOG(LogTemp, Warning, TEXT("UnitController: Move failed or aborted"));
        }
    }

