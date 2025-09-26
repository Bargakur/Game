// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleBuilding.generated.h"

UCLASS()
class GAME_V0_API ASimpleBuilding : public AActor
{
    GENERATED_BODY()
    
public:    
    ASimpleBuilding();
    
    virtual void OnConstruction(const FTransform& Transform) override;
    
    // Building dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float Width;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float Length;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float Height;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float WallThickness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float FloorThickness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
    float RoofThickness;
    
    // Building appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Appearance")
    UMaterialInterface* WallMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Appearance")
    UMaterialInterface* FloorMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Appearance")
    UMaterialInterface* RoofMaterial;
    
    // Building features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Features")
    bool bHasRoof;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Features")
    bool bHasFloor;

    // Door properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Door")
    bool bHasDoor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Door", meta = (EditCondition = "bHasDoor"))
    float DoorWidth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Door", meta = (EditCondition = "bHasDoor"))
    float DoorHeight;

    // Window properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Windows")
    bool bHasWindows;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Windows", meta = (EditCondition = "bHasWindows"))
    float WindowWidth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Windows", meta = (EditCondition = "bHasWindows"))
    float WindowHeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Windows", meta = (EditCondition = "bHasWindows"))
    int32 WindowsPerWall;

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> BuildingComponents;
    
    // Cache static resources
    UPROPERTY()
    UStaticMesh* CubeMesh;
    
    UPROPERTY()
    UMaterialInterface* DefaultMaterial;
    
    void ClearComponents();
    void BuildWalls();
    void BuildFloor();
    void BuildRoof();
    void AddDoor();
    void AddWindows();
};