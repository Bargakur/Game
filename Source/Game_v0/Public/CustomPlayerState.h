// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "buildings/BuildingBase.h"
#include "resource.h"
#include "Net/UnrealNetwork.h"
#include "Race_base.h"
#include "Blueprint/UserWidget.h"
#include "CustomPlayerState.generated.h"

// Forward declarations
class UResourceDisplayWidget;
class AUnitBase;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResourcesChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceSelected);

/**
 * Custom PlayerState for storing team, race, and resource info.
 * Holds replicated game data and fires events for UI updates.
 */
UCLASS()
class GAME_V0_API ACustomPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ACustomPlayerState();

    // Overrides
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Team number: 1 or 2 */
    UPROPERTY(Replicated, BlueprintReadWrite)
    int32 TeamID;

    /** List of player's building types */
    UPROPERTY(Replicated, BlueprintReadWrite)
    TArray<TSubclassOf<ABuildingBase>> PlayerBuildingTypes;

    /** Player's current resources */
    UPROPERTY(Replicated, ReplicatedUsing = OnRep_PlayerResources)
    TArray<FResource> PlayerResources;

    /** The player's race class */
    UPROPERTY(Replicated, BlueprintReadWrite)
    TSubclassOf<URace_base> PlayerRace;

    UPROPERTY(Replicated)
    TArray<AUnitBase*> PlayerUnits;

    UPROPERTY()
    TSubclassOf<URace_base> PlayerRaceClass;

    /** Whether the player has selected a race */
    UPROPERTY(ReplicatedUsing = OnRep_HasSelectedRace, BlueprintReadWrite, Category = "Race")
    bool bHasSelectedRace;

    /** Display name of the player's race */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Race")
    FText RaceDisplayName;

    /** Event: Fired when resources change */
    UPROPERTY(BlueprintAssignable)
    FOnResourcesChanged OnResourcesChanged;

    /** Event: Fired when race is selected (owning client only) */
    UPROPERTY(BlueprintAssignable)
    FOnRaceSelected OnRaceSelected;

    /** Widget class for resource display */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UResourceDisplayWidget> ResourceDisplayWidgetClass;

    // ===== Race Functions =====

    /** Set the player's race (server only) */
    UFUNCTION(BlueprintCallable, Category = "Race")
    void SetPlayerRace(TSubclassOf<URace_base> InRace);

    /** Get the player's race */
    UFUNCTION(BlueprintCallable, Category = "Race")
    TSubclassOf<URace_base> GetPlayerRace() const;

    /** Has the player selected a race? */
    UFUNCTION(BlueprintCallable, Category = "Race")
    bool HasSelectedRace() const { return bHasSelectedRace; }

    // ===== Resource Functions =====

    /** Add a resource (server only) */
    UFUNCTION(Category = "Resources")
    void AddResource(FResource ResourceToAdd);

    /** Check if player has enough of a given resource */
    UFUNCTION(Category = "Resources")
    bool HasEnoughResource(FResource ResourceToCheck, int Amount) const;

    /** Consume a resource (server only) */
    UFUNCTION(Category = "Resources")
    bool ConsumeResource(FResource ResourceToConsume, int Amount);

    /** Get total amount of a resource by name */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 GetTotalResourceAmount(FName ResourceName) const;

    /** Get all unique resource names */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FName> GetUniqueResourceNames() const;

    // Unit management methods
    UFUNCTION(BlueprintCallable)
    void RegisterUnit(AUnitBase* Unit);

    UFUNCTION(BlueprintCallable)
    void UnregisterUnit(AUnitBase* Unit);

    UFUNCTION(BlueprintPure)
    TArray<AUnitBase*> GetPlayerUnits() const { return PlayerUnits; }

    UFUNCTION(BlueprintPure)
    int32 GetUnitCount() const { return PlayerUnits.Num(); }

    UFUNCTION(BlueprintPure)
    TArray<AUnitBase*> GetUnitsOfTeam(int32 InTeamID) const;



protected:
    /** HUD widget instance */
    UPROPERTY()
    UResourceDisplayWidget* ResourceDisplayWidget;

    /** Called when resources replicate */
    UFUNCTION()
    void OnRep_PlayerResources();

    /** Called when race selection replicates */
    UFUNCTION()
    void OnRep_HasSelectedRace();

    /** Broadcasts resource changes to UI and listeners */
    void BroadcastResourceChange();
    

    /** Checks if this PlayerState belongs to the local player */
    bool IsLocalPlayer() const;

    UFUNCTION()
    void InitializeResourcesFromRace();
};
