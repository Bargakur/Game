#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomPlayerState.h"
#include "buildings/CoreBuilding.h"
#include "GameFramework/GameMode.h"
#include "BuildingGameModeDemo.generated.h"

// Structure to define core building spawn points
USTRUCT(BlueprintType)
struct FCoreSpawnPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 TeamID;

    FCoreSpawnPoint()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        TeamID = 0;
    }

    FCoreSpawnPoint(FVector InLocation, FRotator InRotation, int32 InTeamID)
        : Location(InLocation), Rotation(InRotation), TeamID(InTeamID)
    {
    }
};

UCLASS()
class GAME_V0_API ABuildingGameModeDemo : public AGameMode
{
    GENERATED_BODY()

public:
    ABuildingGameModeDemo();

protected:
    virtual void BeginPlay() override;

    // Core building class to spawn
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core Buildings")
    TSubclassOf<ACoreBuilding> CoreBuildingClass;

    // Predefined spawn points for core buildings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Buildings")
    TArray<FCoreSpawnPoint> CoreSpawnPoints;

    // Timer handle for initialization
    FTimerHandle InitializationTimerHandle;

    // Functions
    void SetupPlayerController();
    void SpawnAndSetupCamera();
    void InitializePlayerStateFromGameInstance();
    void SpawnCoreBuildings();

public:
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void OnTeamDefeated(int32 DefeatedTeamID);

    // Helper function to add spawn points via Blueprint or code
    UFUNCTION(BlueprintCallable, Category = "Core Buildings")
    void AddCoreSpawnPoint(FVector Location, FRotator Rotation, int32 TeamID);

    // Helper function to clear all spawn points
    UFUNCTION(BlueprintCallable, Category = "Core Buildings")
    void ClearCoreSpawnPoints();
};