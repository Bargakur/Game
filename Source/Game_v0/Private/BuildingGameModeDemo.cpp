#include "BuildingGameModeDemo.h"
#include "BGameState.h"
#include "BuildingPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABuildingGameModeDemo::ABuildingGameModeDemo()
{
    PlayerControllerClass = ABuildingPlayerController::StaticClass();
    PlayerStateClass = ACustomPlayerState::StaticClass();
    DefaultPawnClass = nullptr;

    // Set default core building class (you can change this in Blueprint or set it to your actual class)
    CoreBuildingClass = ACoreBuilding::StaticClass();

    // Example default spawn points - you can modify these or set them in Blueprint
    // Team 0 spawn points
    CoreSpawnPoints.Add(FCoreSpawnPoint(FVector(-1000.f, -1000.f, 0.f), FRotator::ZeroRotator, 0));
    CoreSpawnPoints.Add(FCoreSpawnPoint(FVector(-1000.f, 1000.f, 0.f), FRotator::ZeroRotator, 0));
    
    // Team 1 spawn points  
    CoreSpawnPoints.Add(FCoreSpawnPoint(FVector(1000.f, -1000.f, 0.f), FRotator::ZeroRotator, 1));
    CoreSpawnPoints.Add(FCoreSpawnPoint(FVector(1000.f, 1000.f, 0.f), FRotator::ZeroRotator, 1));
}

void ABuildingGameModeDemo::BeginPlay()
{
    Super::BeginPlay();
    
    SetupPlayerController();
    SpawnAndSetupCamera();
    SpawnCoreBuildings(); // Spawn core buildings immediately
    
    // Initialize PlayerState from GameInstance after everything is set up
    // Use a short delay to ensure PlayerState is fully initialized
    GetWorldTimerManager().SetTimer(InitializationTimerHandle, this, 
        &ABuildingGameModeDemo::InitializePlayerStateFromGameInstance, 0.1f, false);    
}

void ABuildingGameModeDemo::SetupPlayerController()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("Building Game Mode: No PlayerController found!"));
        return;
    }

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);
}

void ABuildingGameModeDemo::SpawnAndSetupCamera()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    // Default spawn location if no player cores exist
    FVector SpawnLocation(0.f, 0.f, 2000.f);
    FRotator SpawnRotation(-65.f, 0.f, 10.f);
    
    // Get local player's team ID from PlayerState
    int32 LocalPlayerTeamID = 0;
    if (ACustomPlayerState* PS = PC->GetPlayerState<ACustomPlayerState>())
    {
        LocalPlayerTeamID = PS->TeamID;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Building Game Mode: Could not get PlayerState for team ID, defaulting to team 0"));
    }
    
    // Calculate center position of local player's core spawn points
    TArray<FCoreSpawnPoint> LocalPlayerCores;
    for (const FCoreSpawnPoint& SpawnPoint : CoreSpawnPoints)
    {
        if (SpawnPoint.TeamID == LocalPlayerTeamID)
        {
            LocalPlayerCores.Add(SpawnPoint);
        }
    }
    
    if (LocalPlayerCores.Num() > 0)
    {
        // Calculate center of local player's cores
        FVector CenterLocation = FVector::ZeroVector;
        for (const FCoreSpawnPoint& Point : LocalPlayerCores)
        {
            CenterLocation += Point.Location;
        }
        CenterLocation /= LocalPlayerCores.Num();
        
        // Position camera above the center of player's cores
        SpawnLocation = FVector(CenterLocation.X, CenterLocation.Y, CenterLocation.Z + 2000.f);
        
        UE_LOG(LogTemp, Log, TEXT("Building Game Mode: Camera centered above Team %d cores at %s"), 
               LocalPlayerTeamID, *CenterLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Building Game Mode: No cores found for Team %d, using default camera position"), LocalPlayerTeamID);
    }

    // Spawn camera
    ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(), SpawnLocation, SpawnRotation);
    if (Camera)
    {
        
        
        PC->SetViewTarget(Camera);
        UE_LOG(LogTemp, Log, TEXT("Building Game Mode: Top-down camera spawned at location %s"), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Building Game Mode: Failed to spawn Camera!"));
    }
}

void ABuildingGameModeDemo::SpawnCoreBuildings()
{
    if (!CoreBuildingClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Building Game Mode: CoreBuildingClass is not set!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Building Game Mode: World is null!"));
        return;
    }

    for (const FCoreSpawnPoint& SpawnPoint : CoreSpawnPoints)
    {
        FVector AdjustedSpawnLocation = SpawnPoint.Location;
        
        // Method 1: Line trace to find ground level
        FHitResult HitResult;
        FVector TraceStart = SpawnPoint.Location + FVector(0, 0, 1000.f); // Start high above
        FVector TraceEnd = SpawnPoint.Location - FVector(0, 0, 1000.f);   // End below ground
        
        FCollisionQueryParams TraceParams;
        TraceParams.bTraceComplex = false;
        TraceParams.bReturnPhysicalMaterial = false;
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams))
        {
            // Ground found, place building on top
            AdjustedSpawnLocation = HitResult.Location;
        }
        
        // Method 2: Add height offset based on building dimensions
        // Get the default object to check building height
        if (ACoreBuilding* DefaultCore = CoreBuildingClass->GetDefaultObject<ACoreBuilding>())
        {
            float BuildingHeight = DefaultCore->Height;
            if (BuildingHeight <= 0.f)
            {
                // If Height property isn't set, try to get it from the mesh bounds
                if (DefaultCore->BuildingMesh && DefaultCore->BuildingMesh->GetStaticMesh())
                {
                    FBoxSphereBounds Bounds = DefaultCore->BuildingMesh->GetStaticMesh()->GetBounds();
                    BuildingHeight = Bounds.BoxExtent.Z * 2.f; // Full height
                }
                else
                {
                    BuildingHeight = 100.f; // Default fallback height
                }
            }
            
            // Offset the spawn location up by half the building height
            AdjustedSpawnLocation.Z += BuildingHeight * 0.5f;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACoreBuilding* SpawnedCore = World->SpawnActor<ACoreBuilding>(
            CoreBuildingClass,
            AdjustedSpawnLocation,
            SpawnPoint.Rotation,
            SpawnParams
        );

        if (SpawnedCore)
        {
            // Set the team ID for this core building
            SpawnedCore->TeamID = SpawnPoint.TeamID;
            
            // Complete construction immediately (cores start built)
            SpawnedCore->CompleteConstruction();
            
            UE_LOG(LogTemp, Log, TEXT("Building Game Mode: Core building spawned for Team %d at location %s (adjusted from %s)"), 
                   SpawnPoint.TeamID, *AdjustedSpawnLocation.ToString(), *SpawnPoint.Location.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Building Game Mode: Failed to spawn core building for Team %d at location %s"), 
                   SpawnPoint.TeamID, *AdjustedSpawnLocation.ToString());
        }
    }
}

void ABuildingGameModeDemo::InitializePlayerStateFromGameInstance()
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode: InitializePlayerStateFromGameInstance called"));
    
    UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance());
    if (!MyGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: Failed to get MyGameInstance"));
        return;
    }

    if (!MyGameInstance->HasSelectedRace())
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode: No race data in Game Instance"));
        return;
    }

    // Get the player controller and its player state
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: No PlayerController found"));
        return;
    }

    ACustomPlayerState* PS = PC->GetPlayerState<ACustomPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: No CustomPlayerState found"));
        return;
    }

    // Set the race class and resource widget class
    TSubclassOf<URace_base> SelectedRaceClass = MyGameInstance->GetSelectedRaceClass();
    if (SelectedRaceClass)
    {
        // Set the resource widget class in PlayerState before setting race
        if (ABuildingPlayerController* BuildingPC = Cast<ABuildingPlayerController>(PC))
        {
            if (BuildingPC->ResourceHUDClassBP)
            {
                PS->ResourceDisplayWidgetClass = BuildingPC->ResourceHUDClassBP;
                UE_LOG(LogTemp, Log, TEXT("GameMode: Set ResourceDisplayWidgetClass in PlayerState"));
            }
        }
        
        // Now set the race (this will trigger widget creation)
        PS->SetPlayerRace(SelectedRaceClass);
        
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Successfully initialized PlayerState with race: %s"), 
               *SelectedRaceClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: SelectedRaceClass is null"));
    }
}

void ABuildingGameModeDemo::OnTeamDefeated(int32 DefeatedTeamID)
{
    UE_LOG(LogTemp, Warning, TEXT("Team %d has been defeated!"), DefeatedTeamID);

    int32 WinningTeamID = (DefeatedTeamID == 0) ? 1 : 0; // assume just team 0 and team 1

    UE_LOG(LogTemp, Warning, TEXT("Team %d wins the game!"), WinningTeamID);

    EndMatch();
}

void ABuildingGameModeDemo::AddCoreSpawnPoint(FVector Location, FRotator Rotation, int32 TeamID)
{
    CoreSpawnPoints.Add(FCoreSpawnPoint(Location, Rotation, TeamID));
}

void ABuildingGameModeDemo::ClearCoreSpawnPoints()
{
    CoreSpawnPoints.Empty();
}