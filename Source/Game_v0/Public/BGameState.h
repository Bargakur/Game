// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "buildings/CoreBuilding.h"
#include "GameFramework/GameStateBase.h"
#include "BGameState.generated.h"

/**
 * 
 */
UCLASS()
class GAME_V0_API ABGameState : public AGameStateBase
{
	GENERATED_BODY()

	public:

	void RegisterCoreBuilding(ACoreBuilding* Core, int32 TeamID);

	// Called when a core is destroyed
	void NotifyCoreDestroyed(ACoreBuilding* Core, int32 TeamID);

	// Check if all cores of a team are destroyed
	bool AreAllCoresDestroyed(int32 TeamID) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_WinningTeamID();

private:

	UPROPERTY(ReplicatedUsing = OnRep_WinningTeamID)
	int32 WinningTeamId;
	

	TMap<int32, TArray<ACoreBuilding*>> TeamCores;

	UPROPERTY()
	TMap<APlayerState*, ACoreBuilding*> PlayerCores;
};
