#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Race_base.h" // Include your race base class
#include "RaceNames.h"
#include "MyGameInstance.generated.h"


UCLASS()
class GAME_V0_API UMyGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UMyGameInstance();

    void Init();

    void OnStart() override;

    // === Race Selection Functions ===
    
    // Set selected race (enum version - for backwards compatibility)
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetSelectedRace(ERaceNames Race);
    

    
    // Set selected race class (new version for PlayerState integration)
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetSelectedRaceClass(TSubclassOf<URace_base> RaceClass);
    
    // Get selected race class for PlayerState
    UFUNCTION(BlueprintCallable, Category = "Character")
    TSubclassOf<URace_base> GetSelectedRaceClass() const;
    
    // Check if race has been selected
    UFUNCTION(BlueprintCallable, Category = "Character")
    bool HasSelectedRace() const;
    
    // Get selected race enum (for backwards compatibility)
    UFUNCTION(BlueprintCallable, Category = "Character")
    ERaceNames GetSelectedRace() const;

    // === Player Data Functions ===
    
    // Set player name
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetPlayerName(const FString& Name);
    
    // Get player name
    UFUNCTION(BlueprintCallable, Category = "Character")
    FString GetPlayerName() const { return PlayerName; }

    // === Data Management ===
    
    // Reset character-specific data
    UFUNCTION(BlueprintCallable, Category = "Data")
    void ResetCharacterData();
    
    // Reset all game data
    UFUNCTION(BlueprintCallable, Category = "Data")
    void ResetAllGameData();
    
    // Debug function to log current data
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogGameInstanceData();

protected:
    // Player's chosen name
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    FString PlayerName;

    UPROPERTY()
    TMap<ERaceNames, TSubclassOf<URace_base>> RaceClassMap;
    
    // Selected race (enum version)
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ERaceNames SelectedRace;
    
    // Selected race class (for PlayerState)
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    TSubclassOf<URace_base> SelectedRaceClass;
    
    // Has player selected a race?
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    bool bHasSelectedRace;
};