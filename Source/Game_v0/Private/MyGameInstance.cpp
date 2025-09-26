#include "MyGameInstance.h"
#include "Engine/Engine.h"
#include "Dwarves.h"
#include "Elves.h"

UMyGameInstance::UMyGameInstance()
{
    // Initialize default values
    PlayerName = TEXT("");
    SelectedRace = ERaceNames::None; // Assuming you have a None value
    bHasSelectedRace = false;
    SelectedRaceClass = nullptr;
}

void UMyGameInstance::SetSelectedRace(ERaceNames Race)
{
    SelectedRace = Race;
    bHasSelectedRace = true;

    if (TSubclassOf<URace_base>* FoundClass = RaceClassMap.Find(Race))
    {
        SelectedRaceClass = *FoundClass;
        UE_LOG(LogTemp, Warning, TEXT("Game Instance: Selected race class set to %s"),
               *SelectedRaceClass->GetName());
    }
    else
    {
        SelectedRaceClass = nullptr;
        UE_LOG(LogTemp, Error, TEXT("Game Instance: No class mapped for race %s"),
               *UEnum::GetValueAsString(Race));
    }

    UE_LOG(LogTemp, Warning, TEXT("Game Instance: Selected race set to %s"),
           *UEnum::GetValueAsString(Race));
}


void UMyGameInstance::SetSelectedRaceClass(TSubclassOf<URace_base> RaceClass)
{
    SelectedRaceClass = RaceClass;
    bHasSelectedRace = (RaceClass != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("Game Instance: Selected race class set to %s"), 
           RaceClass ? *RaceClass->GetName() : TEXT("None"));
}

TSubclassOf<URace_base> UMyGameInstance::GetSelectedRaceClass() const
{
    return SelectedRaceClass;
}

bool UMyGameInstance::HasSelectedRace() const
{
    return bHasSelectedRace && (SelectedRaceClass != nullptr);
}

ERaceNames UMyGameInstance::GetSelectedRace() const
{
    return SelectedRace;
}

void UMyGameInstance::SetPlayerName(const FString& Name)
{
    PlayerName = Name;
    UE_LOG(LogTemp, Warning, TEXT("Game Instance: Player name set to %s"), *PlayerName);
}

void UMyGameInstance::ResetCharacterData()
{
    PlayerName = TEXT("");
    SelectedRace = ERaceNames::None; // Reset to None
    bHasSelectedRace = false;
    SelectedRaceClass = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Game Instance: Character data reset"));
}

void UMyGameInstance::ResetAllGameData()
{
    ResetCharacterData();
    // Add any other data resets here as you expand the game
    
    UE_LOG(LogTemp, Warning, TEXT("Game Instance: All game data reset"));
}

void UMyGameInstance::LogGameInstanceData()
{
    FString RaceName = UEnum::GetValueAsString(SelectedRace);
    
    UE_LOG(LogTemp, Warning, TEXT("=== Game Instance Data ==="));
    UE_LOG(LogTemp, Warning, TEXT("Selected Race: %s"), *RaceName);
    UE_LOG(LogTemp, Warning, TEXT("Player Name: %s"), *PlayerName);
    UE_LOG(LogTemp, Warning, TEXT("Has Selected Race: %s"), bHasSelectedRace ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("Race Class Set: %s"), SelectedRaceClass ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}
void UMyGameInstance::Init()
{
    Super::Init();
    // Populate enum → class map
#define ADD_RACE_TO_MAP(EnumName, ClassName) \
RaceClassMap.Add(ERaceNames::EnumName, ClassName::StaticClass());

    FOREACH_RACE(ADD_RACE_TO_MAP)

#undef ADD_RACE_TO_MAP
    UE_LOG(LogTemp, Log , TEXT("MyGameInstance initialized"));
}
void UMyGameInstance::OnStart()
{
    Super::OnStart();
    UE_LOG(LogTemp, Log, TEXT("MyGameInstance started"));
    LogGameInstanceData(); // Log current state
}
