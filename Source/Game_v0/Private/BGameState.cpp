// Fill out your copyright notice in the Description page of Project Settings.


#include "BGameState.h"

#include "BuildingGameModeDemo.h"


void ABGameState::RegisterCoreBuilding(ACoreBuilding* Core, int32 TeamID)
{
	TeamCores.FindOrAdd(TeamID).Add(Core);
}

void ::ABGameState::NotifyCoreDestroyed(ACoreBuilding* Core, int32 TeamID)
	{
		// Check defeat
		if (AreAllCoresDestroyed(TeamID))
		{
			if (ABuildingGameModeDemo* GM = GetWorld()->GetAuthGameMode<ABuildingGameModeDemo>())
			{
				GM->OnTeamDefeated(TeamID);
			}
		}
	}
void ABGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(ABGameState, WinningTeamId);
	// Add other replicated properties here if needed
}
void ABGameState::OnRep_WinningTeamID()
{
	
}
bool ABGameState::AreAllCoresDestroyed(int32 TeamID) const
{
	// Find the team's cores in the map
	const TArray<ACoreBuilding*>* TeamCoresPtr = TeamCores.Find(TeamID);
    
	// If no cores were ever registered for this team, consider them "destroyed"
	if (!TeamCoresPtr || TeamCoresPtr->Num() == 0)
	{
		return true;
	}
    
	// Check if all cores in the team are destroyed
	for (ACoreBuilding* Core : *TeamCoresPtr)
	{
		// If any core is still valid, not destroyed, and not pending destruction, team still has cores
		if (IsValid(Core) && !Core->bIsDestroyed && !Core->IsPendingKillPending())
		{
			return false;
		}
	}
    
	// All cores are either null, destroyed, or marked for destruction
	return true;
}
