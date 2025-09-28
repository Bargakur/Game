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
#include "PhysicalResourceActor.h"
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
    if (SelectedRaceClass)
    {
        PS->SetPlayerRace(SelectedRaceClass);
        
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Successfully initialized PlayerState with race: %s"), 
               *SelectedRaceClass->GetName());
               
        // NOW spawn resources since race is initialized
        SpawnInitialResources();
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

FVector ABuildingGameModeDemo::GetRandomLocationNearCores(int32 TeamID, float Radius)
{
    // Find all core spawn points for this team
    TArray<FVector> TeamCoreLocations;
    for (const FCoreSpawnPoint& SpawnPoint : CoreSpawnPoints)
    {
        if (SpawnPoint.TeamID == TeamID)
        {
            TeamCoreLocations.Add(SpawnPoint.Location);
        }
    }

    if (TeamCoreLocations.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No core locations found for Team %d"), TeamID);
        return FVector::ZeroVector;
    }

    // Pick random core location
    FVector BaseLocation = TeamCoreLocations[FMath::RandRange(0, TeamCoreLocations.Num() - 1)];
    
    // Generate random offset within radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(100.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );

    return BaseLocation + Offset;
}

void ABuildingGameModeDemo::SpawnResourcesNearCores(int32 TeamID)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get the player's race class from PlayerState
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    ACustomPlayerState* PlayerState = PC->GetPlayerState<ACustomPlayerState>();
    TSubclassOf<URace_base> RaceClass = PlayerState->GetPlayerRace();
    
    URace_base* RaceInstance = RaceClass->GetDefaultObject<URace_base>();
    if (!RaceInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get race instance for team %d"), TeamID);
        return;
    }

    // Get all initial resource types from the race
    TArray<EResourceKind> ResourceTypes = RaceInstance->GetInitialResourceTypes();
    
    for (EResourceKind ResourceKind : ResourceTypes)
    {
        // Get resource data from race
        FName ResourceName = UGameResources::ResourceKindToName(ResourceKind);
        int32 BaseAmount = RaceInstance->GetInitialResourceAmount(ResourceKind);
        float Weight = RaceInstance->GetInitialResourceWeight(ResourceKind);
        TMap<FName, float> Properties = RaceInstance->GetInitialResourceProperties(ResourceKind);
        
        // Spawn multiple piles of this resource (split the amount)
        int32 NumPiles = FMath::Max(1, BaseAmount / 5); // Create piles of ~5 resources each
        int32 AmountPerPile = FMath::Max(1, BaseAmount / NumPiles);
        int32 RemainingAmount = BaseAmount;
        
        for (int32 i = 0; i < NumPiles && RemainingAmount > 0; i++)
        {
            int32 ThisPileAmount = FMath::Min(AmountPerPile, RemainingAmount);
            RemainingAmount -= ThisPileAmount;
            
            // Get random location near this team's cores
            FVector SpawnLocation = GetRandomLocationNearCores(TeamID, ResourceSpawnRadius);
            
            // Adjust for ground level
            FHitResult HitResult;
            FVector TraceStart = SpawnLocation + FVector(0, 0, 500.f);
            FVector TraceEnd = SpawnLocation - FVector(0, 0, 500.f);
            
            if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                SpawnLocation = HitResult.Location + FVector(0, 0, 50.f); // Slight offset above ground
            }

            // Create resource data
            FResource NewResource(ResourceName, ThisPileAmount, Weight);
            
            // Add properties from race definition
            for (const auto& PropertyPair : Properties)
            {
                NewResource.SetProperty(PropertyPair.Key, PropertyPair.Value);
            }

            // Spawn physical actor
            APhysicalResourceActor* PhysicalResource = World->SpawnActor<APhysicalResourceActor>(
                APhysicalResourceActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator
            );

            if (PhysicalResource)
            {
                // Set up the bidirectional reference
                NewResource.SetOnGround(SpawnLocation);
                NewResource.PhysicalActor = PhysicalResource;
                PhysicalResource->Initialize(NewResource);
                
                UE_LOG(LogTemp, Log, TEXT("Spawned resource %s (Amount: %d, Weight: %.1f) for Team %d at %s"), 
                       *NewResource.ResourceName.ToString(), NewResource.ResourceAmount, 
                       NewResource.Weight, TeamID, *SpawnLocation.ToString());
                       
                // Log properties if any
                if (Properties.Num() > 0)
                {
                    for (const auto& PropertyPair : Properties)
                    {
                        UE_LOG(LogTemp, Log, TEXT("  Property: %s = %.2f"), 
                               *PropertyPair.Key.ToString(), PropertyPair.Value);
                    }
                }
            }
        }
    }
}

void ABuildingGameModeDemo::SpawnInitialResources()
{
    // Check if PlayerState is initialized with a race
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found for resource spawning"));
        return;
    }
    
    ACustomPlayerState* PlayerState = PC->GetPlayerState<ACustomPlayerState>();
    if (!PlayerState || !PlayerState->GetPlayerRace())
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerState or PlayerRace not initialized - cannot spawn resources"));
        return;
    }

    // Get unique team IDs from core spawn points
    TSet<int32> TeamIDs;
    for (const FCoreSpawnPoint& SpawnPoint : CoreSpawnPoints)
    {
        TeamIDs.Add(SpawnPoint.TeamID);
    }

    // Spawn resources for each team using the player's race data
    for (int32 TeamID : TeamIDs)
    {
        SpawnResourcesNearCores(TeamID);
        UE_LOG(LogTemp, Log, TEXT("Spawned initial resources for Team %d"), TeamID);
    }
}