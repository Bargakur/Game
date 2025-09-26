#include "SimpleBuildingSystem.h"

#include "buildings/BuildingBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

// class for TESTING/development


// SimpleBuildingBuilder Implementation
USimpleBuildingBuilder::USimpleBuildingBuilder()
{
    // Initialize with default values
    Width = 400.0f;
    Length = 400.0f;
    Height = 300.0f;

}

USimpleBuildingBuilder* USimpleBuildingBuilder::Create(UWorld* InWorld, const FVector& Location, const FRotator& Rotation)
{
    World = InWorld;
    SpawnLocation = Location;
    SpawnRotation = Rotation;
    return this;
}

USimpleBuildingBuilder* USimpleBuildingBuilder::SetDimensions(float InWidth, float InLength, float InHeight)
{
    Width = InWidth;
    Length = InLength;
    Height = InHeight;
    return this;
}


ABuildingBase* USimpleBuildingBuilder::Build()
{
    if (!World)
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ABuildingBase* Building = World->SpawnActor<ABuildingBase>(SpawnLocation, SpawnRotation, SpawnParams);
    
    if (Building)
    {
        // Set all building properties
        Building->Width = Width;
        Building->Length = Length;
        Building->Height = Height;
        
        // Force reconstruction
        Building->OnConstruction(Building->GetTransform());
    }
    
    return Building;
}
// In SimpleBuildingSystem.cpp

USimpleBuildingBuilder* USimpleBuildingBuilder::SetTeam(int32 InTeamID)
{
    TeamID = InTeamID;
    return this;
}

USimpleBuildingBuilder* USimpleBuildingBuilder::SetMesh(UStaticMesh* InMesh)
{
    BuildingMesh = InMesh;
    return this;
}

void USimpleBuildingBuilder::InitializeBuilding(ABuildingBase* Building)
{
    // Optionally override in derived builder classes
}

