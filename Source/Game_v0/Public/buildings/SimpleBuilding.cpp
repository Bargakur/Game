// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleBuilding.h"

ASimpleBuilding::ASimpleBuilding()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Default values
    Width = 400.0f;
    Length = 400.0f;
    Height = 300.0f;
    WallThickness = 20.0f;
    FloorThickness = 20.0f;
    RoofThickness = 20.0f;
    
    bHasRoof = true;
    bHasFloor = true;
    
    bHasDoor = true;
    DoorWidth = 120.0f;
    DoorHeight = 220.0f;
    
    bHasWindows = true;
    WindowWidth = 100.0f;
    WindowHeight = 100.0f;
    WindowsPerWall = 2;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Find static mesh in constructor
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if(BoxMeshAsset.Succeeded())
    {
        CubeMesh = BoxMeshAsset.Object;
    }
    
    // Find default material in constructor
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if(DefaultMaterialAsset.Succeeded())
    {
        DefaultMaterial = DefaultMaterialAsset.Object;
    }
}

void ASimpleBuilding::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    // Ensure we have the necessary resources
    if (!CubeMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("SimpleBuilding: Cube mesh not found!"));
        return;
    }
    
    // Clear existing components
    ClearComponents();
    
    // Create the building parts
    BuildWalls();
    
    if (bHasFloor)
    {
        BuildFloor();
    }
    
    if (bHasRoof)
    {
        BuildRoof();
    }
    
    if (bHasDoor)
    {
        AddDoor();
    }
    
    if (bHasWindows)
    {
        AddWindows();
    }
}

void ASimpleBuilding::ClearComponents()
{
    for (UStaticMeshComponent* Component : BuildingComponents)
    {
        if (Component)
        {
            Component->DestroyComponent();
        }
    }
    
    BuildingComponents.Empty();
}

void ASimpleBuilding::BuildWalls()
{
    // Front wall
    UStaticMeshComponent* FrontWall = NewObject<UStaticMeshComponent>(this);
    FrontWall->SetStaticMesh(CubeMesh);
    FrontWall->SetRelativeLocation(FVector(Length/2, 0, Height/2));
    FrontWall->SetRelativeScale3D(FVector(WallThickness/100.0f, Width/100.0f, Height/100.0f));
    FrontWall->SetupAttachment(RootComponent);
    FrontWall->RegisterComponent();
    
    if (WallMaterial)
    {
        FrontWall->SetMaterial(0, WallMaterial);
    }
    
    BuildingComponents.Add(FrontWall);
    
    // Back wall
    UStaticMeshComponent* BackWall = NewObject<UStaticMeshComponent>(this);
    BackWall->SetStaticMesh(CubeMesh);
    BackWall->SetRelativeLocation(FVector(-Length/2, 0, Height/2));
    BackWall->SetRelativeScale3D(FVector(WallThickness/100.0f, Width/100.0f, Height/100.0f));
    BackWall->SetupAttachment(RootComponent);
    BackWall->RegisterComponent();
    
    if (WallMaterial)
    {
        BackWall->SetMaterial(0, WallMaterial);
    }
    
    BuildingComponents.Add(BackWall);
    
    // Left wall
    UStaticMeshComponent* LeftWall = NewObject<UStaticMeshComponent>(this);
    LeftWall->SetStaticMesh(CubeMesh);
    LeftWall->SetRelativeLocation(FVector(0, -Width/2, Height/2));
    LeftWall->SetRelativeScale3D(FVector((Length + WallThickness)/100.0f, WallThickness/100.0f, Height/100.0f));
    LeftWall->SetupAttachment(RootComponent);
    LeftWall->RegisterComponent();
    
    if (WallMaterial)
    {
        LeftWall->SetMaterial(0, WallMaterial);
    }
    
    BuildingComponents.Add(LeftWall);
    
    // Right wall
    UStaticMeshComponent* RightWall = NewObject<UStaticMeshComponent>(this);
    RightWall->SetStaticMesh(CubeMesh);
    RightWall->SetRelativeLocation(FVector(0, Width/2, Height/2));
    RightWall->SetRelativeScale3D(FVector((Length + WallThickness)/100.0f, WallThickness/100.0f, Height/100.0f));
    RightWall->SetupAttachment(RootComponent);
    RightWall->RegisterComponent();
    
    if (WallMaterial)
    {
        RightWall->SetMaterial(0, WallMaterial);
    }
    
    BuildingComponents.Add(RightWall);
}

void ASimpleBuilding::BuildFloor()
{
    UStaticMeshComponent* Floor = NewObject<UStaticMeshComponent>(this);
    Floor->SetStaticMesh(CubeMesh);
    Floor->SetRelativeLocation(FVector(0, 0, -FloorThickness/2));
    Floor->SetRelativeScale3D(FVector(Length/100.0f, Width/100.0f, FloorThickness/100.0f));
    Floor->SetupAttachment(RootComponent);
    Floor->RegisterComponent();
    
    if (FloorMaterial)
    {
        Floor->SetMaterial(0, FloorMaterial);
    }
    
    BuildingComponents.Add(Floor);
}

void ASimpleBuilding::BuildRoof()
{
    UStaticMeshComponent* Roof = NewObject<UStaticMeshComponent>(this);
    Roof->SetStaticMesh(CubeMesh);
    Roof->SetRelativeLocation(FVector(0, 0, Height + RoofThickness/2));
    Roof->SetRelativeScale3D(FVector(Length/100.0f, Width/100.0f, RoofThickness/100.0f));
    Roof->SetupAttachment(RootComponent);
    Roof->RegisterComponent();
    
    if (RoofMaterial)
    {
        Roof->SetMaterial(0, RoofMaterial);
    }
    
    BuildingComponents.Add(Roof);
}

void ASimpleBuilding::AddDoor()
{
    // For simplicity, just create a box that represents a door opening
    UStaticMeshComponent* DoorFrame = NewObject<UStaticMeshComponent>(this);
    DoorFrame->SetStaticMesh(CubeMesh);
    DoorFrame->SetRelativeLocation(FVector(Length/2 + 1, 0, DoorHeight/2));
    DoorFrame->SetRelativeScale3D(FVector(WallThickness/50.0f, DoorWidth/100.0f, DoorHeight/100.0f));
    DoorFrame->SetupAttachment(RootComponent);
    DoorFrame->RegisterComponent();
    
    // Using a different material for the door for clarity
    if (DefaultMaterial)
    {
        DoorFrame->SetMaterial(0, DefaultMaterial);
    }
    
    BuildingComponents.Add(DoorFrame);
}

void ASimpleBuilding::AddWindows()
{
    // For simplicity, just create boxes that represent window openings
    // Add windows to the side walls
    float WindowSpacing = Length / (WindowsPerWall + 1);
    
    for (int32 i = 1; i <= WindowsPerWall; ++i)
    {
        // Left wall windows
        UStaticMeshComponent* LeftWindowFrame = NewObject<UStaticMeshComponent>(this);
        LeftWindowFrame->SetStaticMesh(CubeMesh);
        LeftWindowFrame->SetRelativeLocation(FVector(-Length/2 + i * WindowSpacing, -Width/2 - 1, Height/2));
        LeftWindowFrame->SetRelativeScale3D(FVector(WindowWidth/100.0f, WallThickness/50.0f, WindowHeight/100.0f));
        LeftWindowFrame->SetupAttachment(RootComponent);
        LeftWindowFrame->RegisterComponent();
        
        if (DefaultMaterial)
        {
            LeftWindowFrame->SetMaterial(0, DefaultMaterial);
        }
        
        BuildingComponents.Add(LeftWindowFrame);
        
        // Right wall windows
        UStaticMeshComponent* RightWindowFrame = NewObject<UStaticMeshComponent>(this);
        RightWindowFrame->SetStaticMesh(CubeMesh);
        RightWindowFrame->SetRelativeLocation(FVector(-Length/2 + i * WindowSpacing, Width/2 + 1, Height/2));
        RightWindowFrame->SetRelativeScale3D(FVector(WindowWidth/100.0f, WallThickness/50.0f, WindowHeight/100.0f));
        RightWindowFrame->SetupAttachment(RootComponent);
        RightWindowFrame->RegisterComponent();
        
        if (DefaultMaterial)
        {
            RightWindowFrame->SetMaterial(0, DefaultMaterial);
        }
        
        BuildingComponents.Add(RightWindowFrame);
    }
}
