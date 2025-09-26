#include "BuildingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "BuildingSelectionWidget.h"
#include "ResourceDisplayWidget.h"
#include "Blueprint/UserWidget.h"
#include "buildings/Chata.h"
#include "GameFramework/PlayerController.h"
#include "Components.h"
#include "CustomPlayerState.h"
#include "UnitCommand.h"
#include "UnitController.h"
#include "Camera/CameraActor.h"

ABuildingPlayerController::ABuildingPlayerController()
{
    bShowMouseCursor = true; // Enable cursor for UI
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    bIsDraggingCamera = false;
    
    // Load the input mapping context asset directly
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextObject(TEXT("/Game/Input/InputMappingContext1.InputMappingContext1"));
    if (InputMappingContextObject.Succeeded())
    {
        InputMappingContext1 = InputMappingContextObject.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> StartDragObj(TEXT("/Game/Input/actions/IA_StartDrag.IA_StartDrag"));
    if (StartDragObj.Succeeded())
    {
        StartCameraDrag = StartDragObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> StopDragObj(TEXT("/Game/Input/actions/IA_StopDrag.IA_StopDrag"));
    if (StopDragObj.Succeeded())
    {
        StopCameraDrag = StopDragObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> ZoomOutObj(TEXT("/Game/Input/actions/IA_ZoomOut.IA_ZoomOut"));
    if (ZoomOutObj.Succeeded())
    {
        ZoomOut = ZoomOutObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> ZoomInObj(TEXT("/Game/Input/actions/IA_ZoomIn.IA_ZoomIn"));
    if (ZoomInObj.Succeeded())
    {
        ZoomIn = ZoomInObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> BuildPreviewObj(TEXT("/Game/Input/actions/IA_BuildPreview.IA_BuildPreview"));
    if (BuildPreviewObj.Succeeded())
    {
        BuildPreview = BuildPreviewObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> BuildCancelObj(TEXT("/Game/Input/actions/IA_BuildCancel.IA_BuildCancel"));
    if (BuildCancelObj.Succeeded())
    {
        BuildCancel = BuildCancelObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> BuildConfirmObj(TEXT("/Game/Input/actions/IA_BuildConfirm.IA_BuildConfirm"));
    if (BuildConfirmObj.Succeeded())
    {
        BuildConfirm = BuildConfirmObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> DisplayBuildingWidgetObj(TEXT("/Game/Input/actions/DisplayBuildingWidget.DisplayBuildingWidget"));
    if (DisplayBuildingWidgetObj.Succeeded())
    {
        DisplayBuildingWidget = DisplayBuildingWidgetObj.Object;
    }
    
    DefaultBuildingClass = AChata::StaticClass();
    
    // Load widget Blueprint classes
    BuildingSelectionWidgetClass = TSoftClassPtr<UBuildingSelectionWidget>(FSoftObjectPath("/Game/UI/BuildingSelectionWidget.BuildingSelectionWidget_C"));
    
    // Load ResourceDisplayWidget Blueprint class
    ResourceHUDClass = TSoftClassPtr<UResourceDisplayWidget>(FSoftObjectPath("/Game/UI/WBP_ResourceDisplay.WBP_ResourceDisplay_C"));
    // Load Unit Selection Input Actions
    static ConstructorHelpers::FObjectFinder<UInputAction> UnitSelectObj(TEXT("/Game/Input/actions/IA_UnitSelect.IA_UnitSelect"));
    if (UnitSelectObj.Succeeded())
    {
        UnitSelect = UnitSelectObj.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UInputAction> UnitCommandObj(TEXT("/Game/Input/actions/IA_UnitCommand.IA_UnitCommand"));
    if (UnitCommandObj.Succeeded())
    {
        UnitCommand = UnitCommandObj.Object;
    }
    
}

void ABuildingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Setup Input System first
    SetupEnhancedInput();
    
    // Setup Building Placement Component
    SetupBuildingPlacement();
    
    // Load widget classes
    LoadWidgetClasses();
    
    // Handle resource widget creation
    InitializeResourceWidget();

    if (IsLocalController())
    {
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(
            TimerHandle,
            this,
            &ABuildingPlayerController::CreateResourceWidget,
            0.2f,  // short delay to ensure PlayerState initialized
            false
        );
    }
}

void ABuildingPlayerController::SetupEnhancedInput()
{
    
    // Ensure input is enabled
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    
    // Setup Input System
    if (APlayerController* PC = Cast<APlayerController>(this))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = 
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (InputMappingContext1)
            {
                InputSubsystem->AddMappingContext(InputMappingContext1, 0);
                UE_LOG(LogTemp, Log, TEXT("Enhanced input mapping context added successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("InputMappingContext1 is NULL"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get enhanced input system"));
        }
    }
}

void ABuildingPlayerController::SetupBuildingPlacement()
{
    // Attach the building placement component
    BuildingPlacementComponent = NewObject<UBuildingPlacementComponent>(this);
    
    if (BuildingPlacementComponent)
    {
        BuildingPlacementComponent->RegisterComponent();
        BuildingPlacementComponent->SetComponentTickEnabled(true);
        
        // Add this line to set a default building class if you have one
        if (DefaultBuildingClass)
        {
            BuildingPlacementComponent->SetBuildingClass(DefaultBuildingClass);
            UE_LOG(LogTemp, Log, TEXT("Set building class to: %s"), *DefaultBuildingClass->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DefaultBuildingClass is not set in the controller!"));
        }
    }
}

void ABuildingPlayerController::LoadWidgetClasses()
{
    // Load Building Selection Widget
    if (!BuildingSelectionWidgetClass.IsNull())
    {
        BuildingSelectionWidgetBP = BuildingSelectionWidgetClass.LoadSynchronous();
        if (!BuildingSelectionWidgetBP)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load BuildingSelectionWidget Blueprint"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("BuildingSelectionWidget Blueprint loaded successfully"));
        }
    }
    
    // Load Resource Display Widget
    if (!ResourceHUDClass.IsNull())
    {
        ResourceHUDClassBP = ResourceHUDClass.LoadSynchronous();
        if (!ResourceHUDClassBP)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load ResourceDisplayWidget Blueprint"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("ResourceDisplayWidget Blueprint loaded successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ResourceHUDClass is not set - ResourceDisplayWidget will not be available"));
    }
}

void ABuildingPlayerController::InitializeResourceWidget()
{
    ACustomPlayerState* PS = GetPlayerState<ACustomPlayerState>();
    if (!PS)
    {
        // PlayerState not available yet, try again next frame
        UE_LOG(LogTemp, Log, TEXT("PlayerState not available yet, retrying next frame..."));
        GetWorldTimerManager().SetTimerForNextTick(this, &ABuildingPlayerController::InitializeResourceWidget);
        return;
    }
    
    if (PS->bHasSelectedRace)
    {
        // Race already selected, create widget immediately
        CreateResourceWidget();
    }
    else
    {
        // Race not selected yet, bind to the event
        UE_LOG(LogTemp, Log, TEXT("Race not selected yet, binding to OnRaceSelected event"));
        PS->OnRaceSelected.AddDynamic(this, &ABuildingPlayerController::OnRaceSelected);
    }
}

void ABuildingPlayerController::OnRaceSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Race selected! Creating resource widget..."));
    CreateResourceWidget();
}

void ABuildingPlayerController::CreateResourceWidget()
{
    // Don't create if already exists
    if (ResourceHUD && ResourceHUD->IsInViewport())
    {
        UE_LOG(LogTemp, Warning, TEXT("ResourceDisplayWidget already exists and is in viewport"));
        return;
    }
    
    if (!ResourceHUDClassBP)
    {
        UE_LOG(LogTemp, Error, TEXT("ResourceHUDClassBP is null - cannot create ResourceDisplayWidget"));
        return;
    }
    
    ACustomPlayerState* PS = GetPlayerState<ACustomPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerState is null - cannot create ResourceDisplayWidget"));
        return;
    }
    
    // Create the widget from Blueprint class
    ResourceHUD = CreateWidget<UResourceDisplayWidget>(this, ResourceHUDClassBP);
    if (ResourceHUD)
    {
        // Add to viewport with high Z-order to appear on top
        ResourceHUD->AddToViewport(10);
        
        // Update with current resources
        ResourceHUD->UpdateResources(PS->PlayerResources);
        
        UE_LOG(LogTemp, Log, TEXT("ResourceDisplayWidget created and added to viewport successfully"));
        
 
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ResourceDisplayWidget"));
    }
}

void ABuildingPlayerController::OnPlayerResourcesChanged(const TArray<FResource>& UpdatedResources)
{
    if (ResourceHUD && ResourceHUD->IsInViewport())
    {
        ResourceHUD->UpdateResources(UpdatedResources);
        UE_LOG(LogTemp, VeryVerbose, TEXT("ResourceDisplayWidget updated with new resources"));
    }
}

void ABuildingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Camera Controls
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (StartCameraDrag)
        {
            EnhancedInput->BindAction(StartCameraDrag, ETriggerEvent::Triggered, this, &ABuildingPlayerController::HandleStartCameraDrag);
        }
        
        if (StopCameraDrag)
        {
            EnhancedInput->BindAction(StopCameraDrag, ETriggerEvent::Triggered, this, &ABuildingPlayerController::HandleStopCameraDrag);
        }
        
        if (ZoomIn)
        {
            EnhancedInput->BindAction(ZoomIn, ETriggerEvent::Triggered, this, &ABuildingPlayerController::ZoomCameraIn);
        }
        
        if (ZoomOut)
        {
            EnhancedInput->BindAction(ZoomOut, ETriggerEvent::Triggered, this, &ABuildingPlayerController::ZoomCameraOut);
        }
        
        if (BuildPreview)
        {
            EnhancedInput->BindAction(BuildPreview, ETriggerEvent::Triggered, this, &ABuildingPlayerController::StartBuilding);
        }
        
        if (BuildCancel)
        {
            EnhancedInput->BindAction(BuildCancel, ETriggerEvent::Triggered, this, &ABuildingPlayerController::CancelBuilding);
        }
        
        if (BuildConfirm)
        {
            EnhancedInput->BindAction(BuildConfirm, ETriggerEvent::Triggered, this, &ABuildingPlayerController::ConfirmBuilding);
        }
        
        if (DisplayBuildingWidget)
        {
            EnhancedInput->BindAction(DisplayBuildingWidget, ETriggerEvent::Triggered, this, &ABuildingPlayerController::DisplayBuilding);
        }
        if (UnitSelect)
        {
            EnhancedInput->BindAction(UnitSelect, ETriggerEvent::Triggered, this, &ABuildingPlayerController::HandleUnitSelect);
        }
        
        if (UnitCommand)
        {
            EnhancedInput->BindAction(UnitCommand, ETriggerEvent::Triggered, this, &ABuildingPlayerController::HandleUnitCommand);
        }
    }
}

void ABuildingPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsDraggingCamera)
    {
        UpdateCameraDrag();
    }
}

// --- Camera Controls ---
void ABuildingPlayerController::MoveCameraForward(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        float AdjustedSpeed = GetZoomAdjustedMovementSpeed();
        FVector NewLocation = GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * Value * AdjustedSpeed;
        GetPawn()->SetActorLocation(NewLocation);
    }
}

void ABuildingPlayerController::MoveCameraRight(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        float AdjustedSpeed = GetZoomAdjustedMovementSpeed();
        FVector NewLocation = GetPawn()->GetActorLocation() + GetPawn()->GetActorRightVector() * Value * AdjustedSpeed;
        GetPawn()->SetActorLocation(NewLocation);
    }
}

void ABuildingPlayerController::RotateCamera(float Value)
{
    if (Value != 0.0f)
    {
        AddYawInput(Value);
    }
}

void ABuildingPlayerController::ZoomCameraIn()
{
    ACameraActor* Camera = Cast<ACameraActor>(GetViewTarget());
    if (Camera)
    {
        FVector NewLocation = Camera->GetActorLocation() + Camera->GetActorForwardVector() * 150.f;
        Camera->SetActorLocation(NewLocation);
    }
}

void ABuildingPlayerController::ZoomCameraOut()
{
    ACameraActor* Camera = Cast<ACameraActor>(GetViewTarget());
    if (Camera)
    {
        FVector NewLocation = Camera->GetActorLocation() - Camera->GetActorForwardVector() * 150.f;
        Camera->SetActorLocation(NewLocation);
    }
}

// --- Camera Dragging ---
void ABuildingPlayerController::HandleStartCameraDrag()
{
    UE_LOG(LogTemp, Warning, TEXT("Start Camera Drag"));
    bIsDraggingCamera = true;
    GetMousePosition(LastMousePosition.X, LastMousePosition.Y);
}

void ABuildingPlayerController::HandleStopCameraDrag()
{
    UE_LOG(LogTemp, Warning, TEXT("Stop Camera Drag"));
    bIsDraggingCamera = false;
}

void ABuildingPlayerController::UpdateCameraDrag()
{
    FVector2D CurrentMousePosition;
    GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y);

    FVector2D Delta = LastMousePosition - CurrentMousePosition; 

    // Get the camera actor
    ACameraActor* Camera = Cast<ACameraActor>(GetViewTarget());
    if (Camera)
    {
        FVector MoveDirection = FVector(Delta.Y, -Delta.X, 0.f) * 2.5f;
        Camera->SetActorLocation(Camera->GetActorLocation() + MoveDirection); 
    }

    LastMousePosition = CurrentMousePosition;
}

// --- Building Controls ---
void ABuildingPlayerController::StartBuilding()
{
    if (BuildingPlacementComponent)
    {
        BuildingPlacementComponent->StartPlacingBuilding();
    }
}

void ABuildingPlayerController::ConfirmBuilding()
{
    if (BuildingPlacementComponent)
    {
        BuildingPlacementComponent->ConfirmBuildingPlacement();
    }
}

void ABuildingPlayerController::CancelBuilding()
{
    // First check if the building selection widget is open and close it
    if (BuildingSelectionWidgetInstance && BuildingSelectionWidgetInstance->IsInViewport())
    {
        BuildingSelectionWidgetInstance->RemoveFromViewport();
        BuildingSelectionWidgetInstance = nullptr;
        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;
        bIsDraggingCamera = false;
        UE_LOG(LogTemp, Warning, TEXT("Building selection widget closed via cancel"));
        return; // Exit early since we're just closing the widget
    }
    
    // If widget wasn't open, proceed with normal building cancellation
    if (BuildingPlacementComponent)
    {
        BuildingPlacementComponent->CancelBuildingPlacement();
    }
}

void ABuildingPlayerController::DisplayBuilding()
{
    UE_LOG(LogTemp, Warning, TEXT("DisplayBuilding() called"));
    
    // Check if widget is already displayed
    if (BuildingSelectionWidgetInstance && BuildingSelectionWidgetInstance->IsInViewport())
    {
        BuildingSelectionWidgetInstance->RemoveFromViewport();
        BuildingSelectionWidgetInstance = nullptr;
        return;
    }
    
    // Debug log for widget class
    UE_LOG(LogTemp, Warning, TEXT("BuildingSelectionWidgetBP is valid: %s"), 
           BuildingSelectionWidgetBP ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Create and display the widget
    if (BuildingSelectionWidgetBP)
    {
        BuildingSelectionWidgetInstance = CreateWidget<UBuildingSelectionWidget>(this, BuildingSelectionWidgetBP);
        if (BuildingSelectionWidgetInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget instance created successfully"));
            BuildingSelectionWidgetInstance->AddToViewport();
            UE_LOG(LogTemp, Warning, TEXT("Widget added to viewport"));
            
            // Bind to the selection event
            BuildingSelectionWidgetInstance->OnBuildingSelected.AddDynamic(this, &ABuildingPlayerController::OnBuildingClassSelected);
        }
    }
}


void ABuildingPlayerController::OnBuildingClassSelected(TSubclassOf<ABuildingBase> SelectedBuildingClass)
    {
        if (!BuildingPlacementComponent || !SelectedBuildingClass)
        {
            return;
        }
    
        // Cancel any existing building placement first
        if (BuildingPlacementComponent->IsPlacingBuilding())
        {
            UE_LOG(LogTemp, Warning, TEXT("Canceling existing building placement before starting new one"));
            BuildingPlacementComponent->CancelBuildingPlacement();
        }
    
        // Check if player can afford this building before starting placement
        if (!BuildingPlacementComponent->CanPlaceBuilding(SelectedBuildingClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot afford building: %s"), *SelectedBuildingClass->GetName());
        
            // Hide the widget
            if (BuildingSelectionWidgetInstance)
            {
                BuildingSelectionWidgetInstance->RemoveFromViewport();
                BuildingSelectionWidgetInstance = nullptr;
            }
            return; // Don't start placement
        }
    
        BuildingPlacementComponent->SetBuildingClass(SelectedBuildingClass);
        UE_LOG(LogTemp, Warning, TEXT("Building class changed to: %s"), *SelectedBuildingClass->GetName());
    
        // Hide the widget after selection
        if (BuildingSelectionWidgetInstance)
        {
            BuildingSelectionWidgetInstance->RemoveFromViewport();
            BuildingSelectionWidgetInstance = nullptr;
        }
    
        // Start placing the building after selection
        BuildingPlacementComponent->StartPlacingBuilding();
        UE_LOG(LogTemp, Warning, TEXT("Started placing building: %s"), *SelectedBuildingClass->GetName());
    }


// Legacy function kept for compatibility - now just calls CreateResourceWidget
void ABuildingPlayerController::TryDisplayResource()
{
    CreateResourceWidget();
}
float ABuildingPlayerController::GetZoomAdjustedMovementSpeed() const
{
    ACameraActor* Camera = Cast<ACameraActor>(GetViewTarget());
    if (!Camera)
    {
        return 50.f; // Default speed
    }
    
    float BaseSpeed = 50.f;
    
    // Scale speed based on camera height above ground
    FVector CameraLocation = Camera->GetActorLocation();
    float HeightAboveGround = CameraLocation.Z;
    
    // Normalize based on your default height (2000 units)
    // Higher camera = faster movement, lower camera = slower movement
    float SpeedMultiplier = FMath::Clamp(HeightAboveGround / 2000.f, 0.1f, 3.f);
    
    return BaseSpeed * SpeedMultiplier;
}

void ABuildingPlayerController::HandleUnitSelect()
{
    UE_LOG(LogTemp, Log, TEXT("HandleUnitSelect called"));
    
    // Check if we're in building mode - if so, prioritize building system
    if (BuildingPlacementComponent && BuildingPlacementComponent->IsPlacingBuilding())
    {
        UE_LOG(LogTemp, Log, TEXT("In building mode, ignoring unit selection"));
        return;
    }
    
    // Check if building selection widget is open
    if (BuildingSelectionWidgetInstance && BuildingSelectionWidgetInstance->IsInViewport())
    {
        UE_LOG(LogTemp, Log, TEXT("Building selection widget is open, ignoring unit selection"));
        return;
    }
    
    AUnitBase* ClickedUnit = GetUnitUnderCursor();
    if (ClickedUnit)
    {
        UE_LOG(LogTemp, Log, TEXT("Unit found under cursor: %s"), *ClickedUnit->GetName());
        SelectUnit(ClickedUnit);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No unit found under cursor, deselecting all"));
        DeselectAllUnits();
    }
}

void ABuildingPlayerController::HandleUnitCommand()
{
    UE_LOG(LogTemp, Log, TEXT("HandleUnitCommand called"));
    
    // Check if we're in building mode
    if (BuildingPlacementComponent && BuildingPlacementComponent->IsPlacingBuilding())
    {
        UE_LOG(LogTemp, Log, TEXT("In building mode, handling as building confirm"));
        ConfirmBuilding();
        return;
    }
    
    // Check if building selection widget is open
    if (BuildingSelectionWidgetInstance && BuildingSelectionWidgetInstance->IsInViewport())
    {
        UE_LOG(LogTemp, Log, TEXT("Building selection widget is open, ignoring unit command"));
        return;
    }
    
    if (SelectedUnits.Num() > 0)
    {
        FVector TargetLocation = GetWorldLocationUnderCursor();
        if (!TargetLocation.IsZero())
        {
            UE_LOG(LogTemp, Log, TEXT("Commanding %d units to move to %s"), SelectedUnits.Num(), *TargetLocation.ToString());
            CommandSelectedUnits(TargetLocation);
        }
    }
}

void ABuildingPlayerController::SelectUnit(AUnitBase* Unit)
{
    if (!Unit)
    {
        return;
    }
    
    // Deselect all previously selected units
    DeselectAllUnits();
    
    // Select the new unit
    SelectedUnits.Add(Unit);
    Unit->SetIsSelected(true);
    
    UE_LOG(LogTemp, Log, TEXT("Selected unit: %s"), *Unit->GetName());
}

void ABuildingPlayerController::DeselectAllUnits()
{
    for (AUnitBase* Unit : SelectedUnits)
    {
        if (IsValid(Unit))
        {
            Unit->SetIsSelected(false);
        }
    }
    SelectedUnits.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Deselected all units"));
}

void ABuildingPlayerController::CommandSelectedUnits(FVector TargetLocation)
{
    for (AUnitBase* Unit : SelectedUnits)
    {
        if (IsValid(Unit))
        {
            AUnitController* UnitController = Cast<AUnitController>(Unit->GetController());
            if (UnitController)
            {
                FUnitCommand MoveCommand;
                MoveCommand.CommandType = EUnitCommandType::Move;
                MoveCommand.TargetLocation = TargetLocation;
                
                UnitController->ExecuteCommand(MoveCommand);
                UE_LOG(LogTemp, Log, TEXT("Commanded unit %s to move to %s"), *Unit->GetName(), *TargetLocation.ToString());
            }
        }
    }
}

AUnitBase* ABuildingPlayerController::GetUnitUnderCursor()
{
    FVector2D MousePosition;
    if (!GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        return nullptr;
    }
    
    FVector WorldLocation, WorldDirection;
    if (!DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection))
    {
        return nullptr;
    }
    
    // Perform line trace
    FVector TraceStart = WorldLocation;
    FVector TraceEnd = WorldLocation + (WorldDirection * UnitSelectionRange);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(this->GetPawn());
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Pawn,
        QueryParams
    );
    
    if (bHit && HitResult.GetActor())
    {
        AUnitBase* Unit = Cast<AUnitBase>(HitResult.GetActor());
        return Unit;
    }
    
    return nullptr;
}

FVector ABuildingPlayerController::GetWorldLocationUnderCursor()
{
    FVector2D MousePosition;
    if (!GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        return FVector::ZeroVector;
    }
    
    FVector WorldLocation, WorldDirection;
    if (!DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection))
    {
        return FVector::ZeroVector;
    }
    
    // Trace to find ground location
    FVector TraceStart = WorldLocation;
    FVector TraceEnd = WorldLocation + (WorldDirection * UnitSelectionRange);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    // Add selected units to ignored actors
    for (AUnitBase* Unit : SelectedUnits)
    {
        if (IsValid(Unit))
        {
            QueryParams.AddIgnoredActor(Unit);
        }
    }
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    // If no ground hit, project to Z=0 plane
    if (FMath::Abs(WorldDirection.Z) > 0.001f)
    {
        float T = -WorldLocation.Z / WorldDirection.Z;
        if (T > 0)
        {
            return WorldLocation + WorldDirection * T;
        }
    }
    
    return FVector::ZeroVector;
}