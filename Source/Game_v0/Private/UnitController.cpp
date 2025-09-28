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
    
    // Check if movement was successful
    if (Result.Code == EPathFollowingResult::Success)
    {
        // Check if we completed movement for a resource pickup
        if (bHasPendingResourceAction && TargetResourceActor.IsValid())
        {
            CheckResourcePickupCompletion();
        }
    }
    else
    {
        // Movement failed - clear pending resource action
        if (bHasPendingResourceAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("Movement failed for resource pickup, clearing pending action"));
            bHasPendingResourceAction = false;
            TargetResourceActor = nullptr;
        }
    }
    }
void AUnitController::ExecuteResourceCommand(const FUnitCommand& Command)
{
    switch (Command.CommandType)
    {
    case EUnitCommandType::PickupResource:
        HandlePickupResourceCommand(Command);
        break;
            
    case EUnitCommandType::DropResource:
        HandleDropResourceCommand(Command);
        break;
            
    default:
        // Pass other commands to your existing ExecuteCommand method
            ExecuteCommand(Command);
        break;
    }
}
void AUnitController::HandlePickupResourceCommand(const FUnitCommand& Command)
{
    AUnitBase* Unit = Cast<AUnitBase>(GetPawn());
    if (!Unit || !Command.TargetResource.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid unit or target resource for pickup command"));
        return;
    }
    
    APhysicalResourceActor* ResourceActor = Command.TargetResource.Get();
    if (!ResourceActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target resource actor is null"));
        return;
    }
    
    // Check if unit can pick up this resource
    FName ResourceName;
    int32 ResourceAmount;
    float ResourceWeight;
    TMap<FName, float> ResourceProperties;
    ResourceActor->GetResourceData(ResourceName, ResourceAmount, ResourceWeight, ResourceProperties);
    
    FResource TempResource(ResourceName, ResourceAmount, ResourceWeight);
    TempResource.ResourceProperties = ResourceProperties;
    
    if (!Unit->CanPickupResource(TempResource))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unit cannot pickup resource: %s (capacity/weight limit)"), 
               *ResourceName.ToString());
        return;
    }
    
    // Check if we're close enough to pick up immediately
    float DistanceToResource = FVector::Dist(Unit->GetActorLocation(), ResourceActor->GetActorLocation());
    
    if (DistanceToResource <= ResourceInteractionDistance)
    {
        // Close enough - pick up immediately
        Unit->ServerPickupResource(ResourceActor);
        UE_LOG(LogTemp, Log, TEXT("Unit %s picked up resource %s immediately"), 
               *Unit->GetName(), *ResourceName.ToString());
    }
    else
    {
        // Too far - move to resource first
        TargetResourceActor = ResourceActor;
        PendingResourceCommand = Command;
        bHasPendingResourceAction = true;
        
        // Move to the resource location
        FVector TargetLocation = ResourceActor->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("Unit %s moving to pickup resource %s at distance %.1f"), 
               *Unit->GetName(), *ResourceName.ToString(), DistanceToResource);
        
        // Use your existing movement system
        FUnitCommand MoveCommand(EUnitCommandType::Move, TargetLocation);
        ExecuteCommand(MoveCommand);
    }
}

void AUnitController::HandleDropResourceCommand(const FUnitCommand& Command)
{
    AUnitBase* Unit = Cast<AUnitBase>(GetPawn());
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid unit for drop command"));
        return;
    }
    
    if (Command.ResourceSlotIndex >= 0)
    {
        // Drop specific slot
        Unit->ServerDropResource(Command.ResourceSlotIndex);
        UE_LOG(LogTemp, Log, TEXT("Unit %s dropped resource from slot %d"), 
               *Unit->GetName(), Command.ResourceSlotIndex);
    }
    else
    {
        // Drop all resources
        Unit->ServerDropAllResources();
        UE_LOG(LogTemp, Log, TEXT("Unit %s dropped all resources"), *Unit->GetName());
    }
}
void AUnitController::CheckResourcePickupCompletion()
{
    AUnitBase* Unit = Cast<AUnitBase>(GetPawn());
    if (!Unit || !TargetResourceActor.IsValid())
    {
        bHasPendingResourceAction = false;
        return;
    }
    
    APhysicalResourceActor* ResourceActor = TargetResourceActor.Get();
    float DistanceToResource = FVector::Dist(Unit->GetActorLocation(), ResourceActor->GetActorLocation());
    
    if (DistanceToResource <= ResourceInteractionDistance)
    {
        // Close enough - execute the pickup
        Unit->ServerPickupResource(ResourceActor);
        
        FName ResourceName;
        int32 ResourceAmount;
        float ResourceWeight;
        TMap<FName, float> ResourceProperties;
        ResourceActor->GetResourceData(ResourceName, ResourceAmount, ResourceWeight, ResourceProperties);
        
        UE_LOG(LogTemp, Log, TEXT("Unit %s completed movement and picked up resource %s"), 
               *Unit->GetName(), *ResourceName.ToString());
        
        // Clear pending action
        bHasPendingResourceAction = false;
        TargetResourceActor = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unit %s reached destination but still too far from resource (%.1f > %.1f)"), 
               *Unit->GetName(), DistanceToResource, ResourceInteractionDistance);
        
        // Maybe try moving closer or give up
        bHasPendingResourceAction = false;
        TargetResourceActor = nullptr;
    }
}
void AUnitController::CommandPickupNearestResource(FName ResourceName, float SearchRadius)
{
    AUnitBase* Unit = Cast<AUnitBase>(GetPawn());
    if (!Unit)
    {
        return;
    }
    
    APhysicalResourceActor* NearestResource = Unit->FindNearestResource(ResourceName, SearchRadius);
    if (NearestResource)
    {
        FUnitCommand PickupCommand(EUnitCommandType::PickupResource, NearestResource);
        ExecuteResourceCommand(PickupCommand);
        
        UE_LOG(LogTemp, Log, TEXT("Unit %s commanded to pickup nearest %s resource"), 
               *Unit->GetName(), *ResourceName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unit %s could not find any %s resources within %.1f units"), 
               *Unit->GetName(), *ResourceName.ToString(), SearchRadius);
    }
}
void AUnitController::CommandDropAllResources()
{
    FUnitCommand DropCommand(EUnitCommandType::DropResource, -1); // -1 means drop all
    ExecuteResourceCommand(DropCommand);
}

void AUnitController::CommandDropResourceSlot(int32 SlotIndex)
{
    FUnitCommand DropCommand(EUnitCommandType::DropResource, SlotIndex);
    ExecuteResourceCommand(DropCommand);
}