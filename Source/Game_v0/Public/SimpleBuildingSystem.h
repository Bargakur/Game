// SimpleBuildingSystem.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SimpleBuildingSystem.generated.h"

/**
 * Building Builder class
 */
UCLASS()
class GAME_V0_API USimpleBuildingBuilder : public UObject
{
    GENERATED_BODY()
    
public:
    USimpleBuildingBuilder();
    
    // Initialize with default values
    UFUNCTION(BlueprintCallable, Category = "Building Builder")
    USimpleBuildingBuilder* Create(UWorld* World, const FVector& Location, const FRotator& Rotation);
    
    // Building dimension setters
    UFUNCTION(BlueprintCallable, Category = "Building Builder")
    USimpleBuildingBuilder* SetDimensions(float InWidth, float InLength, float InHeight);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties")
    int32 TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties")
    UStaticMesh* BuildingMesh;
    // Internal references
    UPROPERTY()
    UWorld* World;

    UPROPERTY()
    FVector SpawnLocation;

    UPROPERTY()
    FRotator SpawnRotation;
    // Builder pattern methods


    UFUNCTION(BlueprintCallable, Category = "Building")
    USimpleBuildingBuilder* SetTeam(int32 InTeamID);

    UFUNCTION(BlueprintCallable, Category = "Building")
    USimpleBuildingBuilder* SetMesh(UStaticMesh* InMesh);

    
    
    // Build and return the final building
    UFUNCTION(BlueprintCallable, Category = "Building Builder")
    ABuildingBase* Build();
    // Template method to build any building type that derives from BuildingBase
    template<typename T>
    T* BuildSpecific()
    {
        static_assert(TIsDerivedFrom<T, ABuildingBase>::Value, "T must derive from ABuildingBase");
        
        if (!World)
        {
            return nullptr;
        }
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        T* Building = World->SpawnActor<T>(SpawnLocation, SpawnRotation, SpawnParams);
        
        if (Building)
        {
            // Set common properties
            Building->TeamID = TeamID;
            
            // Set the mesh if provided
            if (BuildingMesh)
            {
                Building->SetBuildingMesh(BuildingMesh);
            }
            
            // Allow for custom initialization
            InitializeBuilding(Building);
        }
        
        return Building;
    }
        
private:

    
    // Building properties cache
    float Width;
    float Length;
    float Height;
protected:
    // This can be overridden in subclasses to customize initialization
    virtual void InitializeBuilding(ABuildingBase* Building);

};