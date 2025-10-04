#include "PhysicalResourceActor.h"
#include "ResourceMeshConfig.h"
#include "UnitBase.h"
#include "resource.h"
#include "ResourceMeshSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

APhysicalResourceActor::APhysicalResourceActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Setup root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Setup collision sphere
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetSphereRadius(PickupRadius);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Setup mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // Set default cube mesh path - this will be our fallback
    DefaultResourceMesh = TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube"))
    );
    

    // Scale down to reasonable size
    SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
}

void APhysicalResourceActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bIsInitialized)
    {
        UpdateMeshFromResource();
    }
}

void APhysicalResourceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APhysicalResourceActor, ResourceName);
    DOREPLIFETIME(APhysicalResourceActor, ResourceAmount);
    DOREPLIFETIME(APhysicalResourceActor, ResourceWeight);
    DOREPLIFETIME(APhysicalResourceActor, ResourceProperties);
}

void APhysicalResourceActor::Initialize(const FResource& InResourceData)
{
    ResourceName = InResourceData.ResourceName;
    ResourceAmount = InResourceData.ResourceAmount;
    ResourceWeight = InResourceData.Weight;
    
    // Convert TMap to TArray for replication
    ResourceProperties.Empty();
    for (const auto& PropertyPair : InResourceData.ResourceProperties)
    {
        ResourceProperties.Add(FResourceProperty(PropertyPair.Key, PropertyPair.Value));
    }
    
    bIsInitialized = true;
    
    UpdateMeshFromResource();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicalResourceActor initialized with resource: %s (Amount: %d)"), 
           *ResourceName.ToString(), ResourceAmount);
}

void APhysicalResourceActor::InitializeFromResourceData(FName InResourceName, int32 InAmount, float InWeight, const TMap<FName, float>& InProperties)
{
    ResourceName = InResourceName;
    ResourceAmount = InAmount;
    ResourceWeight = InWeight;
    
    // Convert TMap to TArray for replication - this was missing!
    ResourceProperties.Empty();
    for (const auto& PropertyPair : InProperties)
    {
        ResourceProperties.Add(FResourceProperty(PropertyPair.Key, PropertyPair.Value));
    }
    
    bIsInitialized = true;
    
    UpdateMeshFromResource();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicalResourceActor initialized with resource: %s (Amount: %d)"), 
           *ResourceName.ToString(), ResourceAmount);
}

void APhysicalResourceActor::OnRep_ResourceName()
{
    if (bIsInitialized)
    {
        UpdateMeshFromResource();
    }
}

void APhysicalResourceActor::GetResourceData(FName& OutName, int32& OutAmount, float& OutWeight, TMap<FName, float>& OutProperties) const
{
    OutName = ResourceName;
    OutAmount = ResourceAmount;
    OutWeight = ResourceWeight;
    
    // Convert TArray back to TMap
    OutProperties.Empty();
    for (const FResourceProperty& Property : ResourceProperties)
    {
        OutProperties.Add(Property.PropertyName, Property.PropertyValue);
    }
}

void APhysicalResourceActor::UpdateMeshFromResource()
{
    UE_LOG(LogTemp, Warning, TEXT("=== UpdateMeshFromResource for %s ==="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("ResourceName: %s"), *ResourceName.ToString());
    UE_LOG(LogTemp, Warning, TEXT("bIsInitialized: %s"), bIsInitialized ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("MeshComponent valid: %s"), MeshComponent ? TEXT("TRUE") : TEXT("FALSE"));
    
    if (!MeshComponent || !bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Early return - MeshComponent or not initialized"));
        return;
    }

    UStaticMesh* MeshToUse = nullptr;
    
    // Debug properties
    UE_LOG(LogTemp, Warning, TEXT("Resource Properties Count: %d"), ResourceProperties.Num());
    for (const FResourceProperty& Property : ResourceProperties)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Property: %s = %.2f"), 
               *Property.PropertyName.ToString(), Property.PropertyValue);
    }
    
    // Try to get mesh from ResourceMeshManager
    UResourceMeshManager* MeshMgr = GetMeshManager();
    UE_LOG(LogTemp, Warning, TEXT("MeshManager valid: %s"), MeshMgr ? TEXT("TRUE") : TEXT("FALSE"));
    
    if (MeshMgr)
    {
        // Convert property array back to TMap for mesh manager
        TMap<FName, float> PropertyMap;
        for (const FResourceProperty& Property : ResourceProperties)
        {
            PropertyMap.Add(Property.PropertyName, Property.PropertyValue);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Calling GetMeshForResource with %d properties"), PropertyMap.Num());
        
        TSoftObjectPtr<UStaticMesh> ResourceMesh = MeshMgr->GetMeshForResource(
            ResourceName, PropertyMap
        );
        
        UE_LOG(LogTemp, Warning, TEXT("ResourceMesh path: %s"), *ResourceMesh.ToSoftObjectPath().ToString());
        
        if (!ResourceMesh.ToSoftObjectPath().IsNull())
        {
            UE_LOG(LogTemp, Warning, TEXT("Loading mesh synchronously..."));
            MeshToUse = ResourceMesh.LoadSynchronous();
            if (MeshToUse)
            {
                UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Loaded specific mesh for resource: %s"), 
                       *ResourceName.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FAILED: LoadSynchronous returned null for resource: %s"), 
                       *ResourceName.ToString());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No valid mesh configured for resource: %s"), 
                   *ResourceName.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MeshManager is null, cannot get specific mesh"));
    }
    
    // Fallback to default cube mesh
    if (!MeshToUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("Using fallback cube mesh"));
        if (DefaultResourceMesh.IsValid())
        {
            MeshToUse = DefaultResourceMesh.LoadSynchronous();
            UE_LOG(LogTemp, Warning, TEXT("Fallback cube mesh loaded: %s"), MeshToUse ? TEXT("SUCCESS") : TEXT("FAILED"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DefaultResourceMesh is not valid!"));
        }
    }
    
    // Apply the mesh
    if (MeshToUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting mesh on component: %s"), *MeshToUse->GetName());
        MeshComponent->SetStaticMesh(MeshToUse);

        UE_LOG(LogTemp, Warning, TEXT("Mesh successfully applied to component"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: No mesh to apply - resource will be invisible!"));
    }
    MeshComponent->SetStaticMesh(MeshToUse);
        
    // AUTOMATIC OFFSET CALCULATION based on mesh bounds
    FBox MeshBounds = MeshToUse->GetBoundingBox();
    FVector BoundsCenter = MeshBounds.GetCenter();
    FVector BoundsMin = MeshBounds.Min;
        
    // Calculate offset to place bottom of mesh at Z=0 (ground level)
    // The mesh bounds are relative to the mesh's pivot point
    float ZOffset = -BoundsMin.Z;
        
    UE_LOG(LogTemp, Log, TEXT("Mesh '%s' bounds: Min=(%s), Max=(%s), Center=(%s)"), 
           *MeshToUse->GetName(),
           *MeshBounds.Min.ToString(),
           *MeshBounds.Max.ToString(),
           *BoundsCenter.ToString());
    UE_LOG(LogTemp, Log, TEXT("Calculated Z offset: %.2f to place on ground"), ZOffset);
        
    // Apply the offset
    MeshComponent->SetRelativeLocation(FVector(0, 0, ZOffset));
        
    // Also adjust collision component to be at ground level
    if (CollisionComponent)
    {
        CollisionComponent->SetRelativeLocation(FVector(0, 0, ZOffset));
    }
}

bool APhysicalResourceActor::CanBePickedUpBy(AUnitBase* Unit) const
{
    if (!Unit || !bIsInitialized)
    {
        return false;
    }
    
    // Check if unit has capacity (implement in UnitBase)
    // For now, just return true
    return true;
}

UResourceMeshManager* APhysicalResourceActor::GetMeshManager() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UResourceMeshSubsystem* Subsystem = GameInstance->GetSubsystem<UResourceMeshSubsystem>())
            {
                return Subsystem->GetMeshManager();
            }
        }
    }
    return nullptr;
}