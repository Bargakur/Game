#include "CustomPlayerState.h"

#include "BuildingPlayerController.h"
#include "ResourceDisplayWidget.h"
#include "Engine/World.h"
#include "MyGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "UnitBase.h"

ACustomPlayerState::ACustomPlayerState()
{
    // Initialize variables
    TeamID = 0;
    bHasSelectedRace = false;
    ResourceDisplayWidget = nullptr;
    ResourceDisplayWidgetClass = nullptr;
    
    // Enable replication
    bReplicates = true;
    SetReplicateMovement(false);
}

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACustomPlayerState, TeamID);
    DOREPLIFETIME(ACustomPlayerState, PlayerBuildingTypes);
    DOREPLIFETIME(ACustomPlayerState, PlayerResources);
    DOREPLIFETIME(ACustomPlayerState, PlayerRace);
    DOREPLIFETIME(ACustomPlayerState, bHasSelectedRace);
    DOREPLIFETIME(ACustomPlayerState, RaceDisplayName);
    DOREPLIFETIME(ACustomPlayerState, PlayerUnits);
}

void ACustomPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerState BeginPlay - bHasSelectedRace: %s, IsLocalPlayer: %s"), 
            bHasSelectedRace ? TEXT("true") : TEXT("false"),
            IsLocalPlayer() ? TEXT("true") : TEXT("false"));
    
    // Don't try to get race from Game Instance here anymore - let GameMode handle it
    // The GameMode will call SetPlayerRace after proper initialization
    
    UE_LOG(LogTemp, Log, TEXT("PlayerState BeginPlay complete, waiting for GameMode initialization"));
}

void ACustomPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ACustomPlayerState::SetPlayerRace(TSubclassOf<URace_base> InRace)
{
    UE_LOG(LogTemp, Warning, TEXT("SetPlayerRace called - HasAuthority: %s"), HasAuthority() ? TEXT("true") : TEXT("false"));
    
    if (HasAuthority()) // Only server can set race
    {
        PlayerRace = InRace;
        bHasSelectedRace = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Race set on server: %s"), InRace ? *InRace->GetName() : TEXT("None"));
        
        // Get race display name if race is valid
        if (InRace)
        {
            if (URace_base* RaceInstance = InRace.GetDefaultObject())
            {
                // Assuming your Race_base class has a DisplayName property
                // RaceDisplayName = RaceInstance->DisplayName;
            }
        }
        
        // IMPORTANT: Initialize resources from the race
        InitializeResourcesFromRace();
        
        // Broadcast resource changes after race selection
        BroadcastResourceChange();
        
        UE_LOG(LogTemp, Log, TEXT("Player race set to: %s"), InRace ? *InRace->GetName() : TEXT("None"));
    }
}

TSubclassOf<URace_base> ACustomPlayerState::GetPlayerRace() const
{
    return PlayerRace;
}

void ACustomPlayerState::OnRep_HasSelectedRace()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_HasSelectedRace called - bHasSelectedRace: %s"), 
               bHasSelectedRace ? TEXT("true") : TEXT("false"));
           
    if (bHasSelectedRace)
    {
        // Create HUD immediately for local player when race is replicated
        if (IsLocalPlayer())
        {
            UE_LOG(LogTemp, Log, TEXT("Creating HUD from OnRep_HasSelectedRace"));
        }
        
        // Broadcast the event
        OnRaceSelected.Broadcast();
    }
}


void ACustomPlayerState::AddResource(FResource ResourceToAdd)
{
    if (HasAuthority()) // Only server can modify resources
    {
        // Try to find existing resource with same name AND weight
        bool bFoundExisting = false;
        for (FResource& ExistingResource : PlayerResources)
        {
            if (ExistingResource == ResourceToAdd) // Uses your custom == operator
            {
                ExistingResource.ResourceAmount += ResourceToAdd.ResourceAmount;
                bFoundExisting = true;
                break;
            }
        }
        
        // If no exact match found, add as new resource entry
        if (!bFoundExisting)
        {
            PlayerResources.Add(ResourceToAdd);
        }
        
        BroadcastResourceChange();
    }
}

bool ACustomPlayerState::HasEnoughResource(FResource ResourceToCheck, int amount_arg) const
{
    // Sum all resources with the same name (regardless of weight)
    int32 TotalAmount = 0;
    for (const FResource& Resource : PlayerResources)
    {
        if (Resource.ResourceName == ResourceToCheck.ResourceName)
        {
            TotalAmount += Resource.ResourceAmount;
        }
    }
    return TotalAmount >= amount_arg;
}

bool ACustomPlayerState::ConsumeResource(FResource ResourceToConsume, int amount)
{
    if (!HasAuthority()) // Only server can consume resources
        return false;
    
    // First check if we have enough total resources with this name
    if (!HasEnoughResource(ResourceToConsume, amount))
        return false;
    
    // Consume resources starting from highest weight (best quality first)
    // Sort resources by weight in descending order for consumption
    TArray<FResource*> SameNameResources;
    for (FResource& Resource : PlayerResources)
    {
        if (Resource.ResourceName == ResourceToConsume.ResourceName)
        {
            SameNameResources.Add(&Resource);
        }
    }
    
    // Sort by weight descending (consume highest quality first)
    SameNameResources.Sort([](const FResource& A, const FResource& B) {
        return A.Weight > B.Weight;
    });
    
    int32 RemainingToConsume = amount;
    for (FResource* Resource : SameNameResources)
    {
        if (RemainingToConsume <= 0)
            break;
            
        int32 ConsumeFromThis = FMath::Min(Resource->ResourceAmount, RemainingToConsume);
        Resource->ResourceAmount -= ConsumeFromThis;
        RemainingToConsume -= ConsumeFromThis;
        
        // Remove resource entry if amount reaches zero
        if (Resource->ResourceAmount <= 0)
        {
            PlayerResources.RemoveAll([Resource](const FResource& R) {
                return &R == Resource;
            });
        }
    }
    
    BroadcastResourceChange();
    return true;
}

void ACustomPlayerState::OnRep_PlayerResources()
{
    BroadcastResourceChange();
}

void ACustomPlayerState::BroadcastResourceChange()
{
    UE_LOG(LogTemp, Warning, TEXT("BroadcastResourceChange called - IsLocalPlayer: %s"), IsLocalPlayer() ? TEXT("true") : TEXT("false"));
    
    // Update the widget if it exists
    if (ResourceDisplayWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Updating ResourceDisplayWidget directly"));
        ResourceDisplayWidget->UpdateResources(PlayerResources);
    }
    
    // Also notify the PlayerController's widget
    if (IsLocalPlayer())
    {
        APlayerController* PC = GetPlayerController();
        if (ABuildingPlayerController* BuildingPC = Cast<ABuildingPlayerController>(PC))
        {
            UE_LOG(LogTemp, Warning, TEXT("Notifying PlayerController about resource change"));
            BuildingPC->OnPlayerResourcesChanged(PlayerResources);
        }
    }
    
    // Broadcast the delegate for other systems
    OnResourcesChanged.Broadcast();
}

int32 ACustomPlayerState::GetTotalResourceAmount(FName ResourceName) const
{
    int32 TotalAmount = 0;
    for (const FResource& Resource : PlayerResources)
    {
        if (Resource.ResourceName == ResourceName)
        {
            TotalAmount += Resource.ResourceAmount;
        }
    }
    return TotalAmount;
}

TArray<FName> ACustomPlayerState::GetUniqueResourceNames() const
{
    TArray<FName> UniqueNames;
    for (const FResource& Resource : PlayerResources)
    {
        UniqueNames.AddUnique(Resource.ResourceName);
    }
    return UniqueNames;
}

bool ACustomPlayerState::IsLocalPlayer() const
{
    // Get the player controller associated with this PlayerState
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        return false;
    }
    
    // Check if this player controller is local
    return PC->IsLocalController();
}
void ACustomPlayerState::InitializeResourcesFromRace()
{
    if (!PlayerRace)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerRace is null in InitializeResourcesFromRace"));
        return;
    }
    
    // Clear existing resources
    PlayerResources.Empty();
    
    // Get the race's default object
    URace_base* RaceDefaultObject = PlayerRace.GetDefaultObject();
    if (!RaceDefaultObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get race default object"));
        return;
    }
    
    // Get initial resources directly from the race's maps
    PlayerResources = RaceDefaultObject->GetInitialResourcesArray();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d resources from race maps"), PlayerResources.Num());
    
    // Log the resources we initialized
    for (const FResource& Resource : PlayerResources)
    {
        UE_LOG(LogTemp, Log, TEXT("Initialized resource: %s, Amount: %d, Weight: %f"), 
               *Resource.ResourceName.ToString(), Resource.ResourceAmount, Resource.Weight);
    }
}
void ACustomPlayerState::RegisterUnit(AUnitBase* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to register null unit"));
        return;
    }

    if (!PlayerUnits.Contains(Unit))
    {
        PlayerUnits.Add(Unit);
        UE_LOG(LogTemp, Log, TEXT("Registered unit %s for Team %d (Total units: %d)"), 
               *Unit->GetName(), TeamID, PlayerUnits.Num());
    }
}

void ACustomPlayerState::UnregisterUnit(AUnitBase* Unit)
{
    if (!Unit)
        return;

    if (PlayerUnits.Remove(Unit) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered unit %s from Team %d (Remaining units: %d)"), 
               *Unit->GetName(), TeamID, PlayerUnits.Num());
    }
}
TArray<AUnitBase*> ACustomPlayerState::GetUnitsOfTeam(int32 InTeamID) const
{
    TArray<AUnitBase*> TeamUnits;
    
    for (AUnitBase* Unit : PlayerUnits)
    {
        if (Unit && Unit->GetTeamId() == InTeamID)
        {
            TeamUnits.Add(Unit);
        }
    }
    
    return TeamUnits;
}
