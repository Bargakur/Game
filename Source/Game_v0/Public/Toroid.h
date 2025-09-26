#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Toroid.generated.h"
//Not an actual toroid - discontinous edge connection


USTRUCT(BlueprintType)
struct FToroidalCoordinate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float X;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Y;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Z;

    FToroidalCoordinate()
        : X(0.0f), Y(0.0f), Z(0.0f)
    {
    }

    FToroidalCoordinate(float InX, float InY, float InZ)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    FToroidalCoordinate(const FVector& Vector)
        : X(Vector.X), Y(Vector.Y), Z(Vector.Z)
    {
    }

    FVector ToVector() const
    {
        return FVector(X, Y, Z);
    }
};

USTRUCT(BlueprintType)
struct FWrappedObject
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<AActor> OriginalActor;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> WrappedInstances;

    UPROPERTY()
    FToroidalCoordinate CanonicalPosition;

    FWrappedObject()
    {
    }

    FWrappedObject(AActor* Actor, const FToroidalCoordinate& Position)
        : OriginalActor(Actor), CanonicalPosition(Position)
    {
    }
};

/**
 * Manages a toroidal (wrapping) world where the map edges connect seamlessly
 * Objects near edges are duplicated to maintain visual continuity
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_V0_API UToroidalWorldManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UToroidalWorldManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // World dimensions
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Toroidal World")
    float WorldWidth;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Toroidal World")
    float WorldHeight;

    // Wrapping threshold - objects within this distance from edges will be wrapped
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Toroidal World")
    float WrapThreshold;

    // Center of the toroidal world
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Toroidal World")
    FVector WorldCenter;

    // Objects to track for wrapping
    UPROPERTY()
    TArray<FWrappedObject> TrackedObjects;

    // Objects that should be excluded from wrapping (like terrain, etc.)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Toroidal World")
    TArray<TSubclassOf<AActor>> ExcludedClasses;

    // Coordinate conversion functions
    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    FToroidalCoordinate WorldToToroidal(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    FVector ToroidalToWorld(const FToroidalCoordinate& ToroidalPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    FToroidalCoordinate NormalizeToroidalCoordinate(const FToroidalCoordinate& Coordinate) const;

    // Object management functions
    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    void UpdateActorWrapping(AActor* Actor);

    // Camera management
    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    void WrapCameraPosition(AActor* CameraActor);

    // Distance calculation accounting for wrapping
    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    float GetToroidalDistance(const FVector& Position1, const FVector& Position2) const;

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    FVector GetToroidalDirection(const FVector& From, const FVector& To) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    bool IsActorNearEdge(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    TArray<FVector> GetWrappedPositions(const FVector& OriginalPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Toroidal World")
    void SetWorldDimensions(float Width, float Height);

private:
    // Internal functions
    void CreateWrappedInstances(FWrappedObject& WrappedObject);
    void UpdateWrappedInstances(FWrappedObject& WrappedObject);
    void CleanupWrappedInstances(FWrappedObject& WrappedObject);
    bool ShouldExcludeActor(AActor* Actor) const;
    AActor* CreateWrappedInstance(AActor* OriginalActor, const FVector& Position);

    // Cache for performance
    TMap<AActor*, int32> ActorToIndexMap;
};