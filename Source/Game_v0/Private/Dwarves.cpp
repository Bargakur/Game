// Fill out your copyright notice in the Description page of Project Settings.


#include "Dwarves.h"

#include "buildings/Chata.h"
#include "buildings/DwarfHouse.h"

UDwarves::UDwarves()
{
	// Set race name
	RaceName = FName("Dwarves");

	AvailableBuildings.Add(AChata::StaticClass());
	AvailableBuildings.Add(ADwarfHouse::StaticClass());
    
	// Setup initial resource amounts - these define what resources the race starts with
	InitialResourceAmounts.Add(EResourceKind::wood, 50);
	InitialResourceAmounts.Add(EResourceKind::water_small, 10);
    
	// Setup initial resource weights (per unit) - these define resource quality/density
	InitialResourceWeights.Add(EResourceKind::wood, 2.5f);
	InitialResourceWeights.Add(EResourceKind::water_small, 1.0f);
	
	// The actual starting resources are generated from the maps above
    
	UE_LOG(LogTemp, Log, TEXT("Dwarves race initialized with %d resource types"), 
		   InitialResourceAmounts.Num());
}

UDwarves::~UDwarves()
{
}
