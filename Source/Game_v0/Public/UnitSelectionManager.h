#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCommand.h"
#include "UnitSelectionManager.generated.h"

class AUnitBase;
class AUnitController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, const TArray<AUnitBase*>&, SelectedUnits);

UCLASS()
class GAME_V0_API AUnitSelectionManager : public AActor
{
    GENERATED_BODY()

public:
    AUnitSelectionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Currently selected units
    UPROPERTY()
    TArray<AUnitBase*> SelectedUnits;

    // Player controller that owns this selection manager
    UPROPERTY()
    class APlayerController* OwnerPC;

public:
    // Selection events
    UPROPERTY(BlueprintAssignable)
    FOnSelectionChanged OnSelectionChanged;

    // Selection methods
    UFUNCTION(BlueprintCallable)
    void SelectUnit(AUnitBase* Unit, bool bAddToSelection = false);

    UFUNCTION(BlueprintCallable)
    void SelectUnits(const TArray<AUnitBase*>& Units, bool bAddToSelection = false);

    UFUNCTION(BlueprintCallable)
    void DeselectUnit(AUnitBase* Unit);

    UFUNCTION(BlueprintCallable)
    void DeselectAll();

    // Command methods
    UFUNCTION(BlueprintCallable)
    void IssueCommand(const FUnitCommand& Command);

    UFUNCTION(BlueprintCallable)
    void IssueMoveCommand(FVector TargetLocation);

    UFUNCTION(BlueprintCallable)
    void IssueStopCommand();

    // Query methods
    UFUNCTION(BlueprintPure)
    TArray<AUnitBase*> GetSelectedUnits() const { return SelectedUnits; }

    UFUNCTION(BlueprintPure)
    int32 GetSelectedUnitCount() const { return SelectedUnits.Num(); }

    UFUNCTION(BlueprintPure)
    bool HasSelectedUnits() const { return SelectedUnits.Num() > 0; }

    UFUNCTION(BlueprintPure)
    bool IsUnitSelected(AUnitBase* Unit) const;

    // Initialization
    void SetOwnerPlayerController(APlayerController* PC) { OwnerPC = PC; }

protected:
    void UpdateSelectionVisuals();
    void ValidateSelectedUnits();

    // Formation helpers for multiple unit movement
    TArray<FVector> CalculateFormationPositions(FVector CenterLocation, int32 UnitCount);
};