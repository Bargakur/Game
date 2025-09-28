#pragma once

#include "CoreMinimal.h"
#include "PhysicalResourceActor.h"
#include "UnitCommand.generated.h"

UENUM(BlueprintType)
enum class EUnitCommandType : uint8
{
    None        UMETA(DisplayName = "None"),
    Move        UMETA(DisplayName = "Move"),
    Stop        UMETA(DisplayName = "Stop"),
    Attack      UMETA(DisplayName = "Attack"),
    Patrol      UMETA(DisplayName = "Patrol"),
    PickupResource    UMETA(DisplayName = "Pickup Resource"),
    DropResource      UMETA(DisplayName = "Drop Resource")
};

USTRUCT(BlueprintType)
struct GAME_V0_API FUnitCommand
{
    GENERATED_BODY()

    // Type of command
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
    EUnitCommandType CommandType = EUnitCommandType::None;

    // Target location for movement commands
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
    FVector TargetLocation = FVector::ZeroVector;

    // Target actor for attack/follow commands
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
    TWeakObjectPtr<AActor> TargetActor = nullptr;

    // Command priority (higher = more important)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
    int32 Priority = 0;

    // Constructors
    FUnitCommand()
    {
        CommandType = EUnitCommandType::None;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = 0;
    }

    FUnitCommand(EUnitCommandType InCommandType, FVector InTargetLocation, int32 InPriority = 0)
    {
        CommandType = InCommandType;
        TargetLocation = InTargetLocation;
        TargetActor = nullptr;
        Priority = InPriority;
    }

    FUnitCommand(EUnitCommandType InCommandType, AActor* InTargetActor, int32 InPriority = 0)
    {
        CommandType = InCommandType;
        TargetLocation = InTargetActor ? InTargetActor->GetActorLocation() : FVector::ZeroVector;
        TargetActor = InTargetActor;
        Priority = InPriority;
    }

    // Utility functions
    bool IsValid() const
    {
        return CommandType != EUnitCommandType::None;
    }

    FString ToString() const
    {
        FString TypeString;
        switch (CommandType)
        {
            case EUnitCommandType::Move: TypeString = TEXT("Move"); break;
            case EUnitCommandType::Stop: TypeString = TEXT("Stop"); break;
            case EUnitCommandType::Attack: TypeString = TEXT("Attack"); break;
            case EUnitCommandType::Patrol: TypeString = TEXT("Patrol"); break;
            default: TypeString = TEXT("None"); break;
        }
        
        return FString::Printf(TEXT("Command: %s, Location: %s, Priority: %d"), 
            *TypeString, *TargetLocation.ToString(), Priority);
    }
    
    // NEW: Resource-specific data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Command")
    TWeakObjectPtr<class APhysicalResourceActor> TargetResource = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Command")
    int32 ResourceSlotIndex = -1; // For drop commands

    // Constructor for resource pickup
    FUnitCommand(EUnitCommandType InCommandType, APhysicalResourceActor* InTargetResource)
    {
        CommandType = InCommandType;
        TargetResource = InTargetResource;
        TargetLocation = InTargetResource ? InTargetResource->GetActorLocation() : FVector::ZeroVector;
    }
    
    // Constructor for resource drop
    FUnitCommand(EUnitCommandType InCommandType, int32 InSlotIndex)
    {
        CommandType = InCommandType;
        ResourceSlotIndex = InSlotIndex;
    }
};