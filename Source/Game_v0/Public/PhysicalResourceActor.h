#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalResourceActor.generated.h"

// Forward declaration to avoid circular dependency
struct FResource;

// Replicated struct for resource properties
USTRUCT(BlueprintType)
struct FResourceProperty
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PropertyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PropertyValue;

    FResourceProperty()
    {
        PropertyName = NAME_None;
        PropertyValue = 0.0f;
    }

    FResourceProperty(FName InName, float InValue)
        : PropertyName(InName), PropertyValue(InValue)
    {
    }
};

UCLASS()
class GAME_V0_API APhysicalResourceActor : public AActor
{
    GENERATED_BODY()

public:
    APhysicalResourceActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float PickupRadius = 150.0f;

    // Initialize with resource data
    UFUNCTION(BlueprintCallable)
    void Initialize(const FResource& InResourceData);

    // Initialize with just basic data to avoid circular dependency
    UFUNCTION(BlueprintCallable) 
    void InitializeFromResourceData(FName InResourceName, int32 InAmount, float InWeight, const TMap<FName, float>& InProperties);

    // Check if unit can pick this up
    UFUNCTION(BlueprintCallable)
    bool CanBePickedUpBy(class AUnitBase* Unit) const;

    // Get reference to mesh manager for visuals
    UFUNCTION(BlueprintCallable)
    class UResourceMeshManager* GetMeshManager() const;

    // Get the current resource data
    UFUNCTION(BlueprintCallable)
    void GetResourceData(FName& OutName, int32& OutAmount, float& OutWeight, TMap<FName, float>& OutProperties) const;


protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_ResourceName();

    // Update visual representation
    void UpdateMeshFromResource();
    
    // Default fallback mesh
    UPROPERTY(EditAnywhere, Category = "Defaults")
    TSoftObjectPtr<UStaticMesh> DefaultResourceMesh;

    // Store resource data separately to avoid circular dependency
    UPROPERTY(ReplicatedUsing = OnRep_ResourceName)
    FName ResourceName;
    
    UPROPERTY(Replicated)
    int32 ResourceAmount = 0;
    
    UPROPERTY(Replicated)
    float ResourceWeight = 1.0f;
    
    // Use array of property pairs instead of TMap for replication
    UPROPERTY(Replicated)
    TArray<FResourceProperty> ResourceProperties;

private:
    bool bIsInitialized = false;
};