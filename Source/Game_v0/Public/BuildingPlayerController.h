#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "BuildingPlacementComponent.h"
#include "buildings/BuildingBase.h"
#include "resource.h"
#include "BuildingPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UBuildingSelectionWidget;
class UResourceDisplayWidget;

UCLASS()
class GAME_V0_API ABuildingPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ABuildingPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;

    // Input System
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* InputMappingContext1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
    class UInputAction* UnitSelect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
    class UInputAction* UnitCommand;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* StartCameraDrag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* StopCameraDrag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomIn;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomOut;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* BuildPreview;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* BuildCancel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* BuildConfirm;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DisplayBuildingWidget;

    // Building System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TSubclassOf<class ABuildingBase> DefaultBuildingClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    UBuildingPlacementComponent* BuildingPlacementComponent;

    // Widget Classes (Blueprint references)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSoftClassPtr<UBuildingSelectionWidget> BuildingSelectionWidgetClass;


    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI") 
    TSoftClassPtr<UResourceDisplayWidget> ResourceHUDClass;
protected:

    // Widget Instances
    UPROPERTY()
    UBuildingSelectionWidget* BuildingSelectionWidgetInstance;

    UPROPERTY()
    UResourceDisplayWidget* ResourceHUD;

    // Loaded Blueprint classes
    UPROPERTY()
    UClass* BuildingSelectionWidgetBP;

    // Unit Selection Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Selection")
    TArray<AUnitBase*> SelectedUnits;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Selection")
    float UnitSelectionRange = 5000.0f;

    float GetZoomAdjustedMovementSpeed() const;
public:
    UPROPERTY()
    UClass* ResourceHUDClassBP;

    // Unit Selection Functions
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void HandleUnitSelect();
    
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void HandleUnitCommand();
    
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SelectUnit(AUnitBase* Unit);
    
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void DeselectAllUnits();
    
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void CommandSelectedUnits(FVector TargetLocation);

    UFUNCTION()
    void OnPlayerResourcesChanged(const TArray<FResource>& UpdatedResources);

private:
    // Camera dragging
    bool bIsDraggingCamera;
    FVector2D LastMousePosition;

    AUnitBase* GetUnitUnderCursor();
    FVector GetWorldLocationUnderCursor();


    // Initialization methods
    void SetupEnhancedInput();
    void SetupBuildingPlacement();
    void LoadWidgetClasses();
    void InitializeResourceWidget();

    // Widget management
    void CreateResourceWidget();

    // Camera controls
    UFUNCTION()
    void MoveCameraForward(float Value);
    
    UFUNCTION()
    void MoveCameraRight(float Value);
    
    UFUNCTION()
    void RotateCamera(float Value);
    
    UFUNCTION()
    void ZoomCameraIn();
    
    UFUNCTION()
    void ZoomCameraOut();

    // Camera dragging
    UFUNCTION()
    void HandleStartCameraDrag();
    
    UFUNCTION()
    void HandleStopCameraDrag();
    
    void UpdateCameraDrag();

    // Building controls
    UFUNCTION()
    void StartBuilding();
    
    UFUNCTION()
    void ConfirmBuilding();
    
    UFUNCTION()
    void CancelBuilding();
    
    UFUNCTION()
    void DisplayBuilding();

    // Event handlers
    UFUNCTION()
    void OnBuildingClassSelected(TSubclassOf<ABuildingBase> SelectedBuildingClass);
    
    UFUNCTION()
    void OnRaceSelected();
    
    

    // Legacy method (kept for compatibility)
    UFUNCTION()
    void TryDisplayResource();
};