#pragma once

#include "CoreMinimal.h"
#include "resource.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "UnitBase.generated.h"

class AUnitSelectionManager;

// Add this enum to the base class
UENUM(BlueprintType)
enum class EUnitSex : uint8
{
    Male    UMETA(DisplayName = "Male"),
    Female  UMETA(DisplayName = "Female")
};

UCLASS()
class GAME_V0_API AUnitBase : public ACharacter
{
    GENERATED_BODY()

public:

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
    // Sets default values for this character's properties
    AUnitBase();

    // Appearance (soft reference, not directly loaded at startup)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (AllowedClasses = "SkeletalMesh"))
    TSoftObjectPtr<USkeletalMesh> UnitMesh;

    // Selection visual mesh (soft reference)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (AllowedClasses = "StaticMesh"))
    TSoftObjectPtr<UStaticMesh> SelectionMesh;

    // Selection visual component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
    UStaticMeshComponent* SelectionIndicator;

    // Team and ownership
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamId;

    // Unit stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SeeingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float UnitMovementSpeed = 300.0f;

    // Unit sex (now in base class)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    EUnitSex UnitSex = EUnitSex::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 MaxCarrySlots = 2;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float MaxCarryWeight = 20.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources", ReplicatedUsing = OnRep_CarriedResources)
    TArray<FResource> CarriedResources;

    UFUNCTION(BlueprintCallable)
    bool CanPickupResource(const FResource& Resource) const;
    
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void ServerPickupResource(class APhysicalResourceActor* ResourceActor);
    
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void ServerDropResource(int32 ResourceIndex);
    
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void ServerDropAllResources();
    
    UFUNCTION(BlueprintCallable)
    class APhysicalResourceActor* FindNearestResource(FName ResourceName, float SearchRadius = 1000.0f);
    
    UFUNCTION(BlueprintCallable)
    int32 GetCarriedAmount(FName ResourceName) const;
    
    UFUNCTION(BlueprintCallable)
    bool HasResourceType(FName ResourceName) const;
    
    UFUNCTION(BlueprintCallable)
    float GetCurrentCarryWeight() const;
    
    UFUNCTION()
    void OnRep_CarriedResources();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // Selection state
    UPROPERTY()
    bool bIsSelected;

    // Movement state
    UPROPERTY()
    bool bIsMoving;

    // Selection manager reference
    UPROPERTY()
    AUnitSelectionManager* SelectionManager;

    void UpdateCarriedResourceVisuals();

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Selection methods
    UFUNCTION(BlueprintCallable)
    void SetIsSelected(bool bSelected);

    UFUNCTION(BlueprintPure)
    bool GetIsSelected() const { return bIsSelected; }

    // Movement methods
    UFUNCTION(BlueprintCallable)
    void SetIsMoving(bool bMoving);

    UFUNCTION(BlueprintPure)
    bool GetIsMoving() const { return bIsMoving; }

    // Team methods
    UFUNCTION(BlueprintCallable)
    void SetTeamId(int32 NewTeamId) { TeamId = NewTeamId; }

    UFUNCTION(BlueprintPure)
    int32 GetTeamId() const { return TeamId; }

    // Sex methods
    UFUNCTION(BlueprintCallable)
    void SetUnitSex(EUnitSex NewSex) { UnitSex = NewSex; }

    UFUNCTION(BlueprintPure)
    EUnitSex GetUnitSex() const { return UnitSex; }

    // Initialize unit with team and selection manager
    UFUNCTION(BlueprintCallable)
    void InitializeUnit(int32 InTeamId, AUnitSelectionManager* InSelectionManager);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeed() const { return MovementSpeed; }
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool GetShouldMove() const { return bShouldMove; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimationAsset* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimationAsset* WalkAnimation;

protected:
    // Visual updates
    void UpdateSelectionVisual();
    
    UFUNCTION(BlueprintCallable)
    void UpdateAnimationState();

    // Setup methods
    void SetupCollision();
    void SetupMovement();
    void SetupSelectionIndicator();

    // Asset loading helpers
    void LoadUnitMesh();
    void LoadSelectionMesh();

    // Animation properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    float MovementSpeed = 10.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    bool bShouldMove = false;
};