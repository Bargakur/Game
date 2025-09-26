#include "UnitSelectionManager.h"
#include "UnitBase.h"
#include "UnitController.h"
#include "GameFramework/PlayerController.h"

AUnitSelectionManager::AUnitSelectionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    OwnerPC = nullptr;
}

void AUnitSelectionManager::BeginPlay()
{
    Super::BeginPlay();
}

void AUnitSelectionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up any invalid units from selection
    ValidateSelectedUnits();
}

void AUnitSelectionManager::SelectUnit(AUnitBase* Unit, bool bAddToSelection)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("SelectionManager: Attempted to select null unit"));
        return;
    }

    // Clear previous selection if not adding
    if (!bAddToSelection)
    {
        DeselectAll();
    }

    // Add unit to selection if not already selected
    if (!IsUnitSelected(Unit))
    {
        SelectedUnits.Add(Unit);
        Unit->SetIsSelected(true);
        
        UE_LOG(LogTemp, Log, TEXT("SelectionManager: Selected unit %s"), *Unit->GetName());
    }

    UpdateSelectionVisuals();
    OnSelectionChanged.Broadcast(SelectedUnits);
}

void AUnitSelectionManager::SelectUnits(const TArray<AUnitBase*>& Units, bool bAddToSelection)
{
    if (!bAddToSelection)
    {
        DeselectAll();
    }

    for (AUnitBase* Unit : Units)
    {
        if (Unit && !IsUnitSelected(Unit))
        {
            SelectedUnits.Add(Unit);
            Unit->SetIsSelected(true);
        }
    }

    UpdateSelectionVisuals();
    OnSelectionChanged.Broadcast(SelectedUnits);
    
    UE_LOG(LogTemp, Log, TEXT("SelectionManager: Selected %d units"), Units.Num());
}

void AUnitSelectionManager::DeselectUnit(AUnitBase* Unit)
{
    if (!Unit)
        return;

    SelectedUnits.Remove(Unit);
    Unit->SetIsSelected(false);

    UpdateSelectionVisuals();
    OnSelectionChanged.Broadcast(SelectedUnits);
    
    UE_LOG(LogTemp, Log, TEXT("SelectionManager: Deselected unit %s"), *Unit->GetName());
}

void AUnitSelectionManager::DeselectAll()
{
    for (AUnitBase* Unit : SelectedUnits)
    {
        if (Unit)
        {
            Unit->SetIsSelected(false);
        }
    }

    SelectedUnits.Empty();
    UpdateSelectionVisuals();
    OnSelectionChanged.Broadcast(SelectedUnits);
    
    UE_LOG(LogTemp, Log, TEXT("SelectionManager: Deselected all units"));
}

void AUnitSelectionManager::IssueCommand(const FUnitCommand& Command)
{
    if (SelectedUnits.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SelectionManager: No units selected to receive command"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("SelectionManager: Issuing command to %d units: %s"), 
           SelectedUnits.Num(), *Command.ToString());

    // Handle movement commands with formation
    if (Command.CommandType == EUnitCommandType::Move && SelectedUnits.Num() > 1)
    {
        TArray<FVector> FormationPositions = CalculateFormationPositions(Command.TargetLocation, SelectedUnits.Num());
        
        for (int32 i = 0; i < SelectedUnits.Num() && i < FormationPositions.Num(); i++)
        {
            if (SelectedUnits[i])
            {
                AUnitController* UnitController = Cast<AUnitController>(SelectedUnits[i]->GetController());
                if (UnitController)
                {
                    FUnitCommand FormationCommand = Command;
                    FormationCommand.TargetLocation = FormationPositions[i];
                    UnitController->ExecuteCommand(FormationCommand);
                }
            }
        }
    }
    else
    {
        // Single target or non-movement commands
        for (AUnitBase* Unit : SelectedUnits)
        {
            if (Unit)
            {
                AUnitController* UnitController = Cast<AUnitController>(Unit->GetController());
                if (UnitController)
                {
                    UnitController->ExecuteCommand(Command);
                }
            }
        }
    }
}

void AUnitSelectionManager::IssueMoveCommand(FVector TargetLocation)
{
    FUnitCommand MoveCommand(EUnitCommandType::Move, TargetLocation);
    IssueCommand(MoveCommand);
}

void AUnitSelectionManager::IssueStopCommand()
{
    FUnitCommand StopCommand;
    StopCommand.CommandType = EUnitCommandType::Stop;
    IssueCommand(StopCommand);
}

bool AUnitSelectionManager::IsUnitSelected(AUnitBase* Unit) const
{
    return SelectedUnits.Contains(Unit);
}

void AUnitSelectionManager::UpdateSelectionVisuals()
{
    // Selection visuals will be handled by the units themselves
    // based on their bIsSelected state
}

void AUnitSelectionManager::ValidateSelectedUnits()
{
    // Remove any units that are no longer valid
    SelectedUnits.RemoveAll([](AUnitBase* Unit)
    {
        return !IsValid(Unit);
    });
}

TArray<FVector> AUnitSelectionManager::CalculateFormationPositions(FVector CenterLocation, int32 UnitCount)
{
    TArray<FVector> Positions;
    
    if (UnitCount <= 0)
        return Positions;

    if (UnitCount == 1)
    {
        Positions.Add(CenterLocation);
        return Positions;
    }

    // Simple grid formation
    float Spacing = 150.0f; // Distance between units
    int32 UnitsPerRow = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(UnitCount)));
    
    int32 CurrentUnit = 0;
    for (int32 Row = 0; Row < UnitsPerRow && CurrentUnit < UnitCount; Row++)
    {
        int32 UnitsInThisRow = FMath::Min(UnitsPerRow, UnitCount - CurrentUnit);
        float RowOffset = (UnitsInThisRow - 1) * Spacing * 0.5f;
        
        for (int32 Col = 0; Col < UnitsInThisRow; Col++)
        {
            FVector Position = CenterLocation;
            Position.X += (Col * Spacing) - RowOffset;
            Position.Y += Row * Spacing;
            
            Positions.Add(Position);
            CurrentUnit++;  
        }
    }

    return Positions;
}