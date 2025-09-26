#include "Toroid.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

UToroidalWorldManager::UToroidalWorldManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default world dimensions (adjust these to match your map)
    WorldWidth = 10000.0f;
    WorldHeight = 10000.0f;
    WrapThreshold = 1000.0f; // Objects within 1000 units of edge will be wrapped
    WorldCenter = FVector(0.0f, 0.0f, 0.0f);
}

void UToroidalWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("ToroidalWorldManager initialized with dimensions: %fx%f"), WorldWidth, WorldHeight);
}

void UToroidalWorldManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update all tracked objects
    for (FWrappedObject& WrappedObj : TrackedObjects)
    {
        if (WrappedObj.OriginalActor.IsValid())
        {
            UpdateWrappedInstances(WrappedObj);
        }
    }
    
    // Clean up invalid objects
    TrackedObjects.RemoveAll([](const FWrappedObject& Obj) {
        return !Obj.OriginalActor.IsValid();
    });
}

FToroidalCoordinate UToroidalWorldManager::WorldToToroidal(const FVector& WorldPosition) const
{
    FVector RelativePos = WorldPosition - WorldCenter;
    return FToroidalCoordinate(RelativePos.X, RelativePos.Y, RelativePos.Z);
}

FVector UToroidalWorldManager::ToroidalToWorld(const FToroidalCoordinate& ToroidalPosition) const
{
    return FVector(ToroidalPosition.X, ToroidalPosition.Y, ToroidalPosition.Z) + WorldCenter;
}

FToroidalCoordinate UToroidalWorldManager::NormalizeToroidalCoordinate(const FToroidalCoordinate& Coordinate) const
{
    FToroidalCoordinate Normalized = Coordinate;
    
    // Wrap X coordinate
    float HalfWidth = WorldWidth * 0.5f;
    while (Normalized.X > HalfWidth)
        Normalized.X -= WorldWidth;
    while (Normalized.X < -HalfWidth)
        Normalized.X += WorldWidth;
    
    // Wrap Y coordinate
    float HalfHeight = WorldHeight * 0.5f;
    while (Normalized.Y > HalfHeight)
        Normalized.Y -= WorldHeight;
    while (Normalized.Y < -HalfHeight)
        Normalized.Y += WorldHeight;
    
    return Normalized;
}

void UToroidalWorldManager::RegisterActor(AActor* Actor)
{
    if (!Actor || ShouldExcludeActor(Actor))
    {
        return;
    }
    
    // Check if already registered
    for (const FWrappedObject& Obj : TrackedObjects)
    {
        if (Obj.OriginalActor == Actor)
        {
            return; // Already registered
        }
    }
    
    FToroidalCoordinate ToroidalPos = WorldToToroidal(Actor->GetActorLocation());
    FWrappedObject NewWrappedObject(Actor, ToroidalPos);
    
    int32 Index = TrackedObjects.Add(NewWrappedObject);
    ActorToIndexMap.Add(Actor, Index);
    
    // Create initial wrapped instances if needed
    CreateWrappedInstances(TrackedObjects[Index]);
    
    UE_LOG(LogTemp, Log, TEXT("Registered actor %s for toroidal wrapping"), *Actor->GetName());
}

void UToroidalWorldManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    int32* IndexPtr = ActorToIndexMap.Find(Actor);
    if (IndexPtr)
    {
        int32 Index = *IndexPtr;
        if (TrackedObjects.IsValidIndex(Index))
        {
            CleanupWrappedInstances(TrackedObjects[Index]);
            TrackedObjects.RemoveAt(Index);
        }
        ActorToIndexMap.Remove(Actor);
        
        // Update indices in map
        for (auto& Pair : ActorToIndexMap)
        {
            if (Pair.Value > Index)
            {
                Pair.Value--;
            }
        }
    }
}

void UToroidalWorldManager::UpdateActorWrapping(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    int32* IndexPtr = ActorToIndexMap.Find(Actor);
    if (IndexPtr && TrackedObjects.IsValidIndex(*IndexPtr))
    {
        FWrappedObject& WrappedObj = TrackedObjects[*IndexPtr];
        
        // Update canonical position
        FToroidalCoordinate NewPos = WorldToToroidal(Actor->GetActorLocation());
        WrappedObj.CanonicalPosition = NormalizeToroidalCoordinate(NewPos);
        
        // Update actor position to normalized world position
        FVector NormalizedWorldPos = ToroidalToWorld(WrappedObj.CanonicalPosition);
        if (FVector::Dist(Actor->GetActorLocation(), NormalizedWorldPos) > 0.1f)
        {
            Actor->SetActorLocation(NormalizedWorldPos);
        }
        
        // Update wrapped instances
        UpdateWrappedInstances(WrappedObj);
    }
}

void UToroidalWorldManager::WrapCameraPosition(AActor* CameraActor)
{
    if (!CameraActor)
    {
        return;
    }
    
    FVector CameraPos = CameraActor->GetActorLocation();
    FToroidalCoordinate ToroidalPos = WorldToToroidal(CameraPos);
    FToroidalCoordinate NormalizedPos = NormalizeToroidalCoordinate(ToroidalPos);
    
    FVector NewWorldPos = ToroidalToWorld(NormalizedPos);
    
    // Only update if the position changed significantly
    if (FVector::Dist(CameraPos, NewWorldPos) > 0.1f)
    {
        CameraActor->SetActorLocation(NewWorldPos);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Camera wrapped from %s to %s"), 
               *CameraPos.ToString(), *NewWorldPos.ToString());
    }
}

float UToroidalWorldManager::GetToroidalDistance(const FVector& Position1, const FVector& Position2) const
{
    FToroidalCoordinate Pos1 = NormalizeToroidalCoordinate(WorldToToroidal(Position1));
    FToroidalCoordinate Pos2 = NormalizeToroidalCoordinate(WorldToToroidal(Position2));
    
    // Calculate distance considering wrapping
    float DX = FMath::Abs(Pos1.X - Pos2.X);
    float DY = FMath::Abs(Pos1.Y - Pos2.Y);
    float DZ = FMath::Abs(Pos1.Z - Pos2.Z);
    
    // Consider wrapping for X and Y
    DX = FMath::Min(DX, WorldWidth - DX);
    DY = FMath::Min(DY, WorldHeight - DY);
    
    return FMath::Sqrt(DX * DX + DY * DY + DZ * DZ);
}

FVector UToroidalWorldManager::GetToroidalDirection(const FVector& From, const FVector& To) const
{
    FToroidalCoordinate FromPos = NormalizeToroidalCoordinate(WorldToToroidal(From));
    FToroidalCoordinate ToPos = NormalizeToroidalCoordinate(WorldToToroidal(To));
    
    FVector Direction;
    
    // Calculate X direction considering wrapping
    float DX = ToPos.X - FromPos.X;
    if (FMath::Abs(DX) > WorldWidth * 0.5f)
    {
        DX = DX > 0 ? DX - WorldWidth : DX + WorldWidth;
    }
    Direction.X = DX;
    
    // Calculate Y direction considering wrapping
    float DY = ToPos.Y - FromPos.Y;
    if (FMath::Abs(DY) > WorldHeight * 0.5f)
    {
        DY = DY > 0 ? DY - WorldHeight : DY + WorldHeight;
    }
    Direction.Y = DY;
    
    // Z doesn't wrap
    Direction.Z = ToPos.Z - FromPos.Z;
    
    return Direction.GetSafeNormal();
}

bool UToroidalWorldManager::IsActorNearEdge(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    FToroidalCoordinate ToroidalPos = WorldToToroidal(Actor->GetActorLocation());
    
    float HalfWidth = WorldWidth * 0.5f;
    float HalfHeight = WorldHeight * 0.5f;
    
    bool NearLeftRight = (FMath::Abs(ToroidalPos.X - HalfWidth) < WrapThreshold) || 
                        (FMath::Abs(ToroidalPos.X + HalfWidth) < WrapThreshold);
    bool NearTopBottom = (FMath::Abs(ToroidalPos.Y - HalfHeight) < WrapThreshold) || 
                        (FMath::Abs(ToroidalPos.Y + HalfHeight) < WrapThreshold);
    
    return NearLeftRight || NearTopBottom;
}

TArray<FVector> UToroidalWorldManager::GetWrappedPositions(const FVector& OriginalPosition) const
{
    TArray<FVector> Positions;
    FToroidalCoordinate ToroidalPos = NormalizeToroidalCoordinate(WorldToToroidal(OriginalPosition));
    
    float HalfWidth = WorldWidth * 0.5f;
    float HalfHeight = WorldHeight * 0.5f;
    
    // Check if near edges and add wrapped positions
    bool NearLeft = ToroidalPos.X < (-HalfWidth + WrapThreshold);
    bool NearRight = ToroidalPos.X > (HalfWidth - WrapThreshold);
    bool NearBottom = ToroidalPos.Y < (-HalfHeight + WrapThreshold);
    bool NearTop = ToroidalPos.Y > (HalfHeight - WrapThreshold);
    
    // Right edge wrapping
    if (NearRight)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X -= WorldWidth;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    // Left edge wrapping
    if (NearLeft)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X += WorldWidth;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    // Top edge wrapping
    if (NearTop)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.Y -= WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    // Bottom edge wrapping
    if (NearBottom)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.Y += WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    // Corner wrapping
    if (NearRight && NearTop)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X -= WorldWidth;
        WrappedPos.Y -= WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    if (NearRight && NearBottom)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X -= WorldWidth;
        WrappedPos.Y += WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    if (NearLeft && NearTop)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X += WorldWidth;
        WrappedPos.Y -= WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    if (NearLeft && NearBottom)
    {
        FToroidalCoordinate WrappedPos = ToroidalPos;
        WrappedPos.X += WorldWidth;
        WrappedPos.Y += WorldHeight;
        Positions.Add(ToroidalToWorld(WrappedPos));
    }
    
    return Positions;
}

void UToroidalWorldManager::SetWorldDimensions(float Width, float Height)
{
    WorldWidth = Width;
    WorldHeight = Height;
    
    // Update all existing objects
    for (FWrappedObject& WrappedObj : TrackedObjects)
    {
        UpdateWrappedInstances(WrappedObj);
    }
}

void UToroidalWorldManager::CreateWrappedInstances(FWrappedObject& WrappedObject)
{
    if (!WrappedObject.OriginalActor.IsValid())
    {
        return;
    }
    
    AActor* OriginalActor = WrappedObject.OriginalActor.Get();
    TArray<FVector> WrappedPositions = GetWrappedPositions(OriginalActor->GetActorLocation());
    
    // Clean up existing instances first
    CleanupWrappedInstances(WrappedObject);
    
    // Create new instances
    for (const FVector& Position : WrappedPositions)
    {
        AActor* WrappedInstance = CreateWrappedInstance(OriginalActor, Position);
        if (WrappedInstance)
        {
            WrappedObject.WrappedInstances.Add(WrappedInstance);
        }
    }
}

void UToroidalWorldManager::UpdateWrappedInstances(FWrappedObject& WrappedObject)
{
    if (!WrappedObject.OriginalActor.IsValid())
    {
        return;
    }
    
    AActor* OriginalActor = WrappedObject.OriginalActor.Get();
    
    // Update canonical position
    WrappedObject.CanonicalPosition = NormalizeToroidalCoordinate(WorldToToroidal(OriginalActor->GetActorLocation()));
    
    // Wrap the original actor if it's gone outside bounds
    FVector NormalizedWorldPos = ToroidalToWorld(WrappedObject.CanonicalPosition);
    if (FVector::Dist(OriginalActor->GetActorLocation(), NormalizedWorldPos) > 0.1f)
    {
        OriginalActor->SetActorLocation(NormalizedWorldPos);
    }
    
    // Check if we need to create/update wrapped instances
    if (IsActorNearEdge(OriginalActor))
    {
        CreateWrappedInstances(WrappedObject);
    }
    else
    {
        // Clean up wrapped instances if not near edge
        CleanupWrappedInstances(WrappedObject);
    }
    
    // Update existing wrapped instances
    TArray<FVector> WrappedPositions = GetWrappedPositions(OriginalActor->GetActorLocation());
    
    // Remove invalid instances
    WrappedObject.WrappedInstances.RemoveAll([](const TWeakObjectPtr<AActor>& Instance) {
        return !Instance.IsValid();
    });
    
    // Update positions of valid instances
    for (int32 i = 0; i < WrappedObject.WrappedInstances.Num() && i < WrappedPositions.Num(); ++i)
    {
        if (WrappedObject.WrappedInstances[i].IsValid())
        {
            WrappedObject.WrappedInstances[i]->SetActorLocation(WrappedPositions[i]);
            WrappedObject.WrappedInstances[i]->SetActorRotation(OriginalActor->GetActorRotation());
        }
    }
}

void UToroidalWorldManager::CleanupWrappedInstances(FWrappedObject& WrappedObject)
{
    for (TWeakObjectPtr<AActor>& Instance : WrappedObject.WrappedInstances)
    {
        if (Instance.IsValid())
        {
            Instance->Destroy();
        }
    }
    WrappedObject.WrappedInstances.Empty();
}

bool UToroidalWorldManager::ShouldExcludeActor(AActor* Actor) const
{
    if (!Actor)
    {
        return true;
    }
    
    // Check if actor's class is in the excluded list
    for (const TSubclassOf<AActor>& ExcludedClass : ExcludedClasses)
    {
        if (Actor->IsA(ExcludedClass))
        {
            return true;
        }
    }
    
    return false;
}

AActor* UToroidalWorldManager::CreateWrappedInstance(AActor* OriginalActor, const FVector& Position)
{
    if (!OriginalActor || !GetWorld())
    {
        return nullptr;
    }
    
    // Spawn a duplicate of the original actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Template = OriginalActor;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* WrappedInstance = GetWorld()->SpawnActor<AActor>(
        OriginalActor->GetClass(),
        Position,
        OriginalActor->GetActorRotation(),
        SpawnParams
    );
    
    if (WrappedInstance)
    {
        // Mark as a wrapped instance (you might want to add a component or tag)
        WrappedInstance->Tags.Add(TEXT("WrappedInstance"));
        
        // Disable collision on wrapped instances to prevent gameplay issues
        WrappedInstance->SetActorEnableCollision(false);
        
        // Copy any important properties from the original
        // This might need to be customized based on your specific actor types
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Created wrapped instance of %s at %s"), 
               *OriginalActor->GetName(), *Position.ToString());
    }
    
    return WrappedInstance;
}